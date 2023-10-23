//
// Created by max on 26.09.23.
//

#include "CircleSeparator.h"
#include "AuxGraph.h"

vector<GRBTempConstr> CircleSeparator::SeparateSolution(double *solution, GRBVar *vars) {
  if (config_.inequality_type_ == CircleInequality::TWO_CHORDED) {
	return SeparateTwoChords(solution, vars);
  } else {
	return SeparateHalfChords(solution, vars);
  }
}

vector<GRBTempConstr> CircleSeparator::SeparateTwoChords(double *solution, GRBVar *vars) {
  return {};
}
vector<GRBTempConstr> CircleSeparator::SeparateHalfChords(const double *solution, GRBVar *vars) {
  PLOGD << "Starting search for violated half-chorded-cycle-inequalities.";

  // creating auxiliary graph
  auto aux = HalfChordedAuxGraph(degree_);
  for (int i = 0; i < degree_; ++i) {
	for (int j = 0; j < degree_; ++j) {

	  if (i == j) continue;

	  // add gadgets
	  double gadget_weight = solution[EdgeIndex(i, j)];

	  aux.AddArc(AuxGraph::Node{false, false, i, j},
				 AuxGraph::Node{false, true, i, j},
				 gadget_weight);
	  aux.AddArc(AuxGraph::Node{true, false, i, j},
				 AuxGraph::Node{true, true, i, j},
				 gadget_weight);

	  // add weights for two adjacent edges (i,j) and (j,k)
	  for (int k = 0; k < degree_; ++k) {
		if ((k == i) || (k == j)) continue;

		double triple_weight = 1 - solution[EdgeIndex(i, k)];

		aux.AddArc(AuxGraph::Node{false, true, i, j},
				   AuxGraph::Node{true, false, j, k},
				   triple_weight);
		aux.AddArc(AuxGraph::Node{true, true, i, j},
				   AuxGraph::Node{false, false, j, k},
				   triple_weight);

	  }
	}
  }

  vector<GRBTempConstr> violated_constraints;
  int found{0};

  // check for shortest paths
  for (int i = 0; i < degree_; ++i) {
	for (int j = 0; j < degree_; ++j) {
	  if (i == j) continue;

	  AuxGraph::Node start{false, false, i, j};
	  AuxGraph::Node target{true, false, i, j};

	  auto [cost, path] = aux.Dijkstra(start, target);

	  if (cost < 3) {
		// right-hand-side of the constraint is dependent on the length of the circle in the original graph, which is
		// half the length of the shortest path found
		int k = (path.size() - 1) / 2;

		GRBLinExpr constraint_lhs;

		// every edge in this shortest path corresponds to a variable in the constraint
		for (int index = 1; index < path.size(); ++index) {
		  auto node1 = path[index - 1];
		  auto node2 = path[index];

		  // inside a gadget, i.e. a positive edge in the violated constraint
		  if (node1.i == node2.i && node1.j == node2.j) {
			constraint_lhs -= vars[EdgeIndex(node1.i, node1.j)];
		  }
			// trans-gadget edge, i.e. a negative edge in the violated constraint
			// because the auxiliary graph was build by adding (i,j) and (j,k),
			// we now know that this corresponds to the edge (i,k) in the original graph
		  else if (node1.j == node2.i) {
			constraint_lhs += vars[EdgeIndex(node1.i, node2.j)];
		  }
		}

		auto constraint_rhs = k - 3;

		violated_constraints.emplace_back(constraint_lhs <= constraint_rhs);
		found++;

		PLOGV << "Found violated half-chorded cycle inequality: " << constraint_lhs << "<= " << constraint_rhs << ".";
	  }
	}
  }
  PLOGD << "Found " << found << " violated half-chorded-cycle-inequalities.";
  return violated_constraints;
}
