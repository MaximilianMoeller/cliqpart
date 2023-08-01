//
// Created by max on 26.05.23.
//

#include "triangle_separator.h"
#include <gurobi_c++.h>
#include <algorithm>

typedef tuple<double, tuple<int, int, int>> TriangleTuple;

int TriangleSeparator::AddCuts() {
  vector<TriangleTuple> triangles;
  // only add up to MAXCUT violated inequalities in one iteration
  int violated{0};

  // iterate through whole graph to find a violated triangle inequalities
  // we only look at the inequality -x_ij + x_ik + x_jk <= 1 here
  // and deal with the other inequalities by iterating over all pairs {(i,j,k) \in \Z_{degree}^3}
  for (int i = 2; i < model_.NodeCount(); ++i) {
	// enumerate about 5 * maxcut violated inequalities
	if (config_.maxcut_ > 0 && violated >= 5 * config_.maxcut_) break;

	for (int j = 1; j < i; ++j) {

	  auto v_ij = model_.GetSolution(i, j);

	  // if x_ij is already 1 or more (numerical inaccuracies) the inequality can no longer be violated -> continue
	  if (v_ij >= 1.0) continue;

	  for (int k = 0; k < j; ++k) {

		auto v_ik = model_.GetSolution(i, k);
		auto v_jk = model_.GetSolution(j, k);

		double violation_degree = -v_ij + v_ik + v_jk - 1;

		// only add triangles if they violate the corresponding inequality by more than the tolerance
		// prevents an issue where, due to floating point arithmetic, the same inequality would be added over and over again
		//
		// examples for tolerance = 0.05 (smaller in reality)
		// | v_ij  | v_ik  | v_jk  | violation_degree  | note
		// | ===== | ===== | ===== | ================= | ================
		// | 1     | 1     | 1     | 0				   | needs to be considered satisfied
		// | 0.999 | 1     | 1     | 1.001 - 1 = 0.001 | should be considered satisfied
		// | 0.990 | 1     | 1     | 1.010 - 1 = 0.010 | should be considered violated
		if (violation_degree > config_.tolerance_) {
		  triangles.emplace_back(-v_ij + v_ik + v_jk, make_tuple(i, j, k));
		  violated++;
		}
	  }
	}
  }
  PLOGD << "Enumerated " << violated << " violated Δ-inequalities";

  int constraints_added{0};

  if (!triangles.empty()) {

	// to prevent the graph from being ‘partially solved’ in one area before any inequality of another area is even considered,
	// configurable via the variable_once_ attribute in the run config
	std::vector<std::vector<bool>> in_inequality;
	if (config_.variable_once_) {
	  in_inequality = vector(model_.NodeCount(), std::vector<bool>(model_.NodeCount(), false));
	};


	// sort vector by degree of violation
	std::sort(triangles.begin(),
			  triangles.end(),
			  [](TriangleTuple a, TriangleTuple b) { return std::get<0>(a) > std::get<0>(b); });

	for (auto &triangle : triangles) {
	  if (config_.maxcut_ > 0 && constraints_added >= config_.maxcut_) break;
	  auto [i, j, k] = std::get<1>(triangle);

	  if (config_.variable_once_) {
		if (in_inequality[i][j] || in_inequality[i][k] || in_inequality[j][k]) continue;

		in_inequality[i][j] = in_inequality[j][i]
			= in_inequality[i][k] = in_inequality[k][i]
			= in_inequality[j][k] = in_inequality[k][j] = true;
	  }

	  model_.addConstr(-model_.GetVar(i, j) + model_.GetVar(i, k) + model_.GetVar(j, k) <= 1);
	  constraints_added++;

	}
	PLOGD << "Added " << constraints_added << " Δ-inequalities";
  }
  return constraints_added;
}