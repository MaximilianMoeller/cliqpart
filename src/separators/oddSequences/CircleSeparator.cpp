//
// Created by max on 26.09.23.
//

#include "CircleSeparator.h"
#include "AuxGraph.h"

vector<GRBTempConstr> CircleSeparator::SeparateSolution(double *solution, GRBVar *vars) {

  vector<GRBTempConstr> res;
  AuxGraph aux{degree_};

  // construct auxiliary graph
  for (int i = 0; i < degree_; ++i) {
    for (int j = 0; j < degree_; ++j) {
      if (i == j) continue;

      // for every edge {i,j} in the original graph, add a gadget consisting of 4 nodes
      double gadget_weight;
      switch (config_.inequality_type_) {
        case CircleInequality::TWO_CHORDED: {
          gadget_weight = -solution[EdgeIndex(i, j)];
        }
          break;
        case CircleInequality::HALF_CHORDED: {
          gadget_weight = 1 - solution[EdgeIndex(i, j)];
        }
          break;
      }

      aux.AddArc(AuxGraph::Node{false, false, i, j},
                 AuxGraph::Node{false, true, i, j},
                 gadget_weight);
      aux.AddArc(AuxGraph::Node{true, false, i, j},
                 AuxGraph::Node{true, true, i, j},
                 gadget_weight);

      for (int k = 0; k < degree_; ++k) {

        if ((k == i) || (k == j)) continue;

        double transitive_weight;
        switch (config_.inequality_type_) {
          case CircleInequality::TWO_CHORDED: {
            transitive_weight = solution[EdgeIndex(i, k) + 1 / 2];
          }
            break;
          case CircleInequality::HALF_CHORDED: {
            transitive_weight = solution[EdgeIndex(i, k)];
          }
            break;
        }

        aux.AddArc(AuxGraph::Node{false, true, i, j},
                   AuxGraph::Node{true, false, j, k},
                   transitive_weight);
      }
    }
  }

  aux.FloydWarshall();

  double comparison;
  switch (config_.inequality_type_) {
    case CircleInequality::TWO_CHORDED: {
      comparison = 1 / 2;
    }
      break;
    case CircleInequality::HALF_CHORDED: {
      comparison = 3;
    }
      break;
  }

  for (int j = 0; j < degree_; ++j) {
    for (int i = 0; i < j; ++i) {
      // i is smaller than j
      AuxGraph::Node start{false, false, i, j};
      AuxGraph::Node target{true, false, i, j};

      double cost = aux.getWeight(start, target);

      if (cost < comparison) {
        auto path = aux.Path(start, target);
        PLOGI << "Found violated inequality!!!";
        for (auto n : path) {
          PLOGV << "(" << n.uv << ", " << n.in_out << ", " << n.i << ", " << n.j << ")";
        }

      }

    }
  }

  return res;
}
