//
// Created by max on 14.06.23.
//

#include "st_separator.h"
#include <algorithm>
#include <vector>
#include <random>

int ST_Separator::add_Cuts() {
  int cuts_added {0};

  // "For every node v \in V_n, we do the following […]"
  for (int v = 0; v < node_count_; ++v) {
	if (cuts_added >= maxcut_) break;

	// "We set W := { w \in V_n \{v} | 0 < v_vw < 1 } […]"
	vector<int> W;
	for (int w = 0; w < node_count_; ++w) {
	  if (w == v) continue;
	  auto v_vw = model_.GetSolution(v, w);
	  if (0.0 + tolerance < v_vw && v_vw < 1.0 - tolerance) {
		W.emplace_back(w);
	  }
	}
	if (W.empty()) continue;

	// "[…] and choose some ordering of the nodes of W"
	std::shuffle(W.begin(), W.end(), std::mt19937(std::random_device()()));

	vector<int> T{};

	// "[…] set T := {w}"
	// "[…] and for every node i \in W\{w}"
	for (auto i : W) {

	  // "[…] if v_ij = 0 for all j \in T"
	  bool add_i {true};
	  for (auto j : T) {
		if (i == j) continue;
		if (model_.GetSolution(i, j) >= tolerance) {
		  add_i = false;
		  break;
		}
	  }
	  if (add_i) {
		// "[…] T := T u {i}"
		T.emplace_back(i);
	  }
	}
	double sum{0};
	for_each(T.begin(), T.end(), [&](int n) {
	  sum += model_.GetSolution(v, n);
	});

	if (sum > 1 + tolerance) {
	  // add all the terms to the constraints
	  GRBLinExpr constraint;
	  for_each(T.begin(), T.end(), [&](int n) {
		constraint += model_.GetVar(v, n);
	  });
	  model_.addConstr(constraint <= 1);
	  cuts_added++;
	}
  }
  PLOGD << "Added " << cuts_added << " [S:T]-inequalities";
  return cuts_added;
};