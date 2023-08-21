//
// Created by max on 14.06.23.
//

#include "st_separator.h"
#include <algorithm>
#include <vector>
#include <random>

vector<GRBTempConstr> StSeparator::SeparateSolution(double *solution, GRBVar *vars) {
  int cuts_added{0};
  vector<GRBTempConstr> result;

  PLOGV << "Starting enumeration of st-inequalities.";

  // "For every node v \in V_n, we do the following […]"
  for (int v = 0; v < degree_; ++v) {
	if (cuts_added >= config_.maxcut_) break;

	// "We set W := { w \in V_n \{v} | 0 < v_vw < 1 } […]"
	vector<int> w_1;
	for (int w = 0; w < degree_; ++w) {
	  if (w == v) continue;
	  auto v_vw = solution[EdgeIndex(v, w)];
	  if (0.0 + config_.tolerance_ < v_vw && v_vw < 1.0 - config_.tolerance_) {
		w_1.emplace_back(w);
	  }
	}
	if (w_1.empty()) continue;

	// "[…] and choose some ordering of the nodes of W"
	//std::shuffle(w_1.begin(), w_1.end(), std::mt19937(std::random_device()()));

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
			  if (solution[EdgeIndex(kI, j)] >= config_.tolerance_) {
				add_i = false;
				break;
			  }
			}
		  }
			break;

		  case StSeparatorHeuristic::GW2: {
			auto sum_ij{0.0};
			for (auto j : t) {
			  sum_ij += solution[EdgeIndex(kI, j)];
			}
			if (solution[EdgeIndex(kI, v)] - sum_ij < config_.tolerance_) add_i = false;
		  }
			break;
		}

		// add the first element of W
		if (add_i || t.empty()) {
		  // "[…] T := T u {i}"
		  t.emplace_back(kI);
		}
	  }
	  if (t.size() == 1) {
		continue;
	  }
	  double sum{0};
	  for (auto n : t) {
		auto index = EdgeIndex(v, n);
		sum += solution[index];
	  }

	  if (sum > 1 + config_.tolerance_) {
		// add all positive terms to the constraints
		GRBLinExpr constraint;
		for (auto n : t) {
		  constraint += vars[EdgeIndex(v, n)];
		};
		// add all negative terms, i.e. the edges in T
		for (auto first = t.begin(); first != t.end(); ++first) {
		  for (auto second = first + 1; second != t.end(); ++second) {
			constraint -= vars[EdgeIndex(*first, *second)];
		  }
		}

		PLOGV << "Found violated constraint: " << constraint << "<= 1";
		result.emplace_back(constraint <= 1);
		cuts_added++;
	  }
	}
  }
  PLOGD << "Enumerated " << cuts_added << " violated [S:T]-inequalities";
  return result;
};