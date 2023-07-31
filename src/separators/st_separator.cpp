//
// Created by max on 14.06.23.
//

#include "st_separator.h"
#include <algorithm>
#include <vector>
#include <random>

int StSeparator::AddCuts() {
  int cuts_added{0};

  // "For every node v \in V_n, we do the following […]"
  for (int v = 0; v < model_.NodeCount(); ++v) {
	if (cuts_added >= config_.maxcut_) break;

	// "We set W := { w \in V_n \{v} | 0 < v_vw < 1 } […]"
	vector<int> w_1;
	for (int w = 0; w < model_.NodeCount(); ++w) {
	  if (w == v) continue;
	  auto v_vw = model_.GetSolution(v, w);
	  if (0.0 + config_.tolerance_ < v_vw && v_vw < 1.0 - config_.tolerance_) {
		w_1.emplace_back(w);
	  }
	}
	if (w_1.empty()) continue;

	// "[…] and choose some ordering of the nodes of W"
	std::shuffle(w_1.begin(), w_1.end(), std::mt19937(std::random_device()()));

	// "[…] We repeat this procedure with converse ordering of W"
	vector<int> w_2{w_1};
	std::reverse(w_2.begin(), w_2.end());
	vector<vector<int>> ws{w_1, w_2};

	for (const auto &kW : ws) {
	  vector<int> t{};
	  // "[…] set T := {w}"
	  // "[…] and for every node i \in W\{w}"
	  for (auto &kI : kW) {

		// "[…] if v_ij = 0 for all j \in T"
		bool add_i{true};

		switch (config_.heuristic_) {

		  case StSeparatorHeuristic::GW1: {
			for (auto j : t) {
			  if (kI == j) continue;
			  if (model_.GetSolution(kI, j) >= config_.tolerance_) {
				add_i = false;
				break;
			  }
			}
		  }
			break;

		  case StSeparatorHeuristic::GW2: {
			auto sum_ij{0.0};
			for (auto j : t) {
			  if (kI == j) continue;
			  sum_ij += model_.GetSolution(kI, j);
			}
			if (model_.GetSolution(kI, v) - sum_ij < config_.tolerance_) add_i = false;
		  }
			break;
		}

		if (add_i) {
		  // "[…] T := T u {i}"
		  t.emplace_back(kI);
		}
	  }
	  double sum{0};
	  for_each(t.begin(), t.end(), [&](int n) {
		sum += model_.GetSolution(v, n);
	  });

	  if (sum > 1 + config_.tolerance_) {
		// add all the terms to the constraints
		GRBLinExpr constraint;
		for_each(t.begin(), t.end(), [&](int n) {
		  constraint += model_.GetVar(v, n);
		});
		model_.addConstr(constraint <= 1);
		cuts_added++;
	  }
	}
  }
  PLOGD << "Added " << cuts_added << " [S:T]-inequalities";
  return cuts_added;
};