//
// Created by max on 26.05.23.
//

#include "triangle_separator.h"
#include <gurobi_c++.h>
#include <iostream>

using namespace std;

void TriangleSeparator::my_callback() {
  try {
	if (where == GRB_CB_MIPSOL) {

	  // iterate through whole graph to find a violated triangle
	  for (int i = 2; i < GetDegree(); ++i) {
		for (int j = 1; j < i; ++j) {
		  // if the edge is in the CP, i.e. x_ij=1, no triangle inequality can
		  // be violated -> continue
		  auto v_ij = getSolution(i, j);
		  if (v_ij > 0.9)
			continue;

		  for (int k = 0; k < j; ++k) {
			// GetIndex the values of the other two the edge decisions
			auto v_ik = getSolution(i, k);
			auto v_jk = getSolution(j, k);

			// a triangle inequality is violated iff the sum of the other edges
			// in the triangle is 2; i.e. edge ij is not in the clique
			// partitioning while edges ik and jk are.
			double sum = v_ik + v_jk;
			// gurobi might return values slightly fractional
			if (1.75 < sum && sum < 2.25) {
			  addLazy(-GetVar(i, j) + GetVar(i, k) + GetVar(j, k) <= 1);
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
	cout << "Error during my_callback" << endl;
  }
}