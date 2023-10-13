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

  // check for shortest paths
  for (int i = 0; i < degree_; ++i) {
    for (int j = 0; j < degree_; ++j) {
      if (i == j) continue;

      AuxGraph::Node start{false, false, i, j};
      AuxGraph::Node target{true, false, i, j};

      auto [cost, path] = aux.Dijkstra(start, target);
      PLOGV << "(i, j, cost): (" << i << ", " << j << ", " << cost << ").";

      if (cost < 3) {
        PLOGI << "Found violated inequality!!!";
        string p{""};
        for (auto n : path) {
          p.append("(" + string(n.uv ? "v" : "u") + ", " + string(n.in_out ? "1" : "0") + ", " + to_string(n.i) + ", "
                       + to_string(n.j) + ")");
        }
        PLOGI << p;
      }
    }
  }
  return {};
}
