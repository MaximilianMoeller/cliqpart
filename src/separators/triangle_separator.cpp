//
// Created by max on 26.05.23.
//

#include "triangle_separator.h"
#include <gurobi_c++.h>
#include <algorithm>

using namespace std;

typedef tuple<double, GRBVar, GRBVar, GRBVar> triangle_tuple;

int TriangleSeparator::add_Cuts() {
	int degree = model_.GetDegree();
	vector<triangle_tuple> triangles;

	// only add up to MAXCUT violated inequalities in one iteration
	int violated{0};

	// iterate through whole graph to find a violated triangle inequalities
	for (int i = 2; i < degree; ++i) {
	  // enumerate about 5 * maxcut violated inequalities
	  if (violated >= 5 * maxcut_) break;
	  for (int j = 1; j < i; ++j) {
		auto v_ij = model_.GetSolution(i, j);

		for (int k = 0; k < j; ++k) {

		  auto v_ik = model_.GetSolution(i, k);
		  auto v_jk = model_.GetSolution(j, k);

		  if (v_ij + v_ik - v_jk > 1) {
			triangles.emplace_back(v_ij + v_ik - v_jk, model_.getVar(i, j), model_.getVar(i, k), model_.getVar(j, k));
			violated++;
		  }
		}
	  }
	}
	PLOGD << "Enumerated " << violated << " violated Δ-inequalities";

	int constraints_added {0};

	if (!triangles.empty()) {

	  // to prevent the graph from being ‘partially solved’ in one area before any inequality of another area is even considered,
	  // I will only add one inequality containing a given edge per iteration
	  bool **in_inequality = new bool *[degree];
	  for (int i = 0; i < degree; i++) in_inequality[i] = new bool[degree]{false};

	  // sort vector by degree of violation
	  std::sort(triangles.begin(),
				triangles.end(),
				[](triangle_tuple a, triangle_tuple b) { return std::get<0>(a) > std::get<0>(b); });

	  for (auto &triangle : triangles) {
		if (constraints_added >= maxcut_) break;
		auto x_ij = get<1>(triangle);
		auto x_ik = get<2>(triangle);
		auto x_jk = get<3>(triangle);

		model_.addConstr(x_ij + x_ik - x_jk <= 1);
		constraints_added++;

	  }
	  PLOGD << "Added " << constraints_added << " Δ-inequalities";

	  for (int i = 0; i < degree; i++) delete[] in_inequality[i];
	  delete[] in_inequality;
	}
  return constraints_added;
}