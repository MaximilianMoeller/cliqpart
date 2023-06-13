//
// Created by max on 26.05.23.
//

#include "cubic_triangle_separator.h"
#include <gurobi_c++.h>
#include <iostream>

using namespace std;

void CubicTriangleSeparator::callback() {
  try {
	if (where == GRB_CB_MIPSOL) {

	  // iterate through whole graph to find a violated triangle
	  for (int i = 0; i < graph_.GetDegree(); ++i) {
		for (int j = 0; j < i; ++j) {
		  // if the edge is in the CP, i.e. x_ij=1, no triangle inequality can
		  // be violated -> continue
		  auto x_ij = graph_.GetVar(i, j);
		  auto v_ij = getSolution(x_ij);
		  if (v_ij > 0.9)
			continue;

		  for (int k = 0; k < j; ++k) {
			// Get the values of the other two the edge decisions
			auto x_ik = graph_.GetVar(i, k);
			auto v_ik = getSolution(x_ik);
			auto x_jk = graph_.GetVar(j, k);
			auto v_jk = getSolution(x_jk);

			// a triangle inequality is violated iff the sum of the other edges
			// in the triangle is 2; i.e. edge ij is not in the clique
			// partitioning while edges ik and jk are.
			double sum = v_ik + v_jk;
			// gurobi might return values slightly fractional
			if (1.75 < sum && sum < 2.25) {
			  addLazy(-x_ij + x_ik + x_jk <= 1);
			}
		  }
		}
	  }
	}
  } catch (GRBException &e) {
	cout << "Error number: " << e.

		getErrorCode()

		 << endl;
	cout << e.

		getMessage()

		 << endl;
  } catch (...) {
	cout << "Error during callback" << endl;
  }
}