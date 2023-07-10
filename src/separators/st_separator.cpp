//
// Created by max on 14.06.23.
//

#include <algorithm>
#include "st_separator.h"
typedef tuple<double, GRBVar, GRBVar, GRBVar> triangle_tuple;

void ST_Separator::my_callback() {
  // in integral solutions only triangle inequalities are separated
  if (where == GRB_CB_MIPSOL) {

	int degree = GetDegree();
	vector<triangle_tuple> triangles;

	// only add up to MAXCUT violated inequalities in one iteration
	int violated{0};

	// iterate through whole graph to find a violated triangle inequalities
	for (int i = 2; i < degree; ++i) {
	  // enumerate about 5 * maxcut violated inequalities
	  if (violated >= 5 * maxcut_) break;
	  for (int j = 1; j < i; ++j) {
		auto v_ij = getSolution(i, j);

		for (int k = 0; k < j; ++k) {

		  auto v_ik = getSolution(i, k);
		  auto v_jk = getSolution(j, k);

		  if (v_ij + v_ik - v_jk > 1) {
			triangles.emplace_back(v_ij + v_ik - v_jk, GetVar(i, j), GetVar(i, k), GetVar(j, k));
			violated++;
		  }
		}
	  }
	}
	PLOGD << "Enumerated " << violated << " violated Δ-inequalities";

	if (!triangles.empty()) {

	  violated = 0;

	  // to prevent the graph from being ‘partially solved’ in one area before any inequality of another area is even considered,
	  // I will only add one inequality containing a given edge per iteration
	  bool **in_inequality = new bool *[degree];
	  for (int i = 0; i < degree; i++) in_inequality[i] = new bool[degree]{false};

	  // sort vector by degree of violation
	  std::sort(triangles.begin(),
				triangles.end(),
				[](triangle_tuple a, triangle_tuple b) { return std::get<0>(a) > std::get<0>(b); });

	  for (auto &triangle : triangles) {
		if (violated >= maxcut_) break;
		auto x_ij = get<1>(triangle);
		auto x_ik = get<2>(triangle);
		auto x_jk = get<3>(triangle);

		// TODO: dummkopf, du addest gerade cuts für alle dreiecke, die du oben in diesen vector steckst. xD
		addLazy(x_ij + x_ik - x_jk <= 1);
		violated++;

	  }
	  PLOGD << "Added " << violated << " Δ-inequalities";
	  //if (in_inequality[i][j]) continue;
	  //if (in_inequality[i][k] || in_inequality[j][k]) continue;


	  for (int i = 0; i < degree; i++) delete[] in_inequality[i];
	  delete[] in_inequality;
	  return;
	}

  }
};