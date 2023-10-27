//
// Created by max on 26.09.23.
//

#include "CircleSeparator.h"
#include "AuxGraph.h"
#include "progressbar.hpp"

vector<GRBTempConstr> CircleSeparator::SeparateSolution(double *solution, GRBVar *vars) {

  // what kind of auxiliary graph is used
  unique_ptr<AuxGraph> aux{};

  // how the weights in the auxiliary graph are set
  std::function<double(int, int)> gadget_weight;
  std::function<double(int, int)> triple_weight;


  // the minimum length of a path in the auxiliary graph such that the corresponding inequality is not violated
  double min_path_cost;
  // what coefficients the variables corresponding to hops inside a gadget should get
  // notice that between_gadget_coefficient = -1 * gadget_coefficient
  int gadget_coefficient;
  // how the rhs of the constraint should be build depending on the circle length
  std::function<int(int)> constraint_rhs;

  // switch only once and prepare everything such that the rest of the code is the same for
  // two-chorded and half-chorded odd cycle inequalities
  switch (config_.inequality_type_) {

    case CircleInequality::TWO_CHORDED: {
      aux = make_unique<TwoChordedAuxGraph>(degree_);

      gadget_weight = [this, &solution](int a, int b) { return 0.5 - solution[EdgeIndex(a, b)]; };
      triple_weight = [this, &solution](int a, int b) { return solution[EdgeIndex(a, b)]; };

      min_path_cost = 0.5;
      gadget_coefficient = 1;
      constraint_rhs = [](int k) { return (k - 1) / 2; };
    }
      break;
    case CircleInequality::HALF_CHORDED: {
      aux = make_unique<HalfChordedAuxGraph>(degree_);

      gadget_weight = [this, &solution](int a, int b) { return solution[EdgeIndex(a, b)]; };
      triple_weight = [this, &solution](int a, int b) { return 1 - solution[EdgeIndex(a, b)]; };

      min_path_cost = 3;
      gadget_coefficient = -1;
      constraint_rhs = [](int k) { return k - 3; };
    }
      break;
  }
  PLOGD << "Starting search for violated " << config_.inequality_type_ << " odd cycle inequalities.";

  // built auxiliary graph
  for (int i = 0; i < degree_; ++i) {
    for (int j = 0; j < degree_; ++j) {

      if (i == j) continue;

      aux->AddArc(AuxGraph::Node{false, false, i, j}, AuxGraph::Node{false, true, i, j}, gadget_weight(i, j));
      aux->AddArc(AuxGraph::Node{true, false, i, j}, AuxGraph::Node{true, true, i, j}, gadget_weight(i, j));

      // add weights for two adjacent edges (i,j) and (j,k)
      for (int k = 0; k < degree_; ++k) {
        if ((k == i) || (k == j)) continue;

        aux->AddArc(AuxGraph::Node{false, true, i, j}, AuxGraph::Node{true, false, j, k}, triple_weight(i, k));
        aux->AddArc(AuxGraph::Node{true, true, i, j}, AuxGraph::Node{false, false, j, k}, triple_weight(i, k));

      }
    }
  }

  // search auxiliary graph for a violated constraint
  int found{0};
  vector<GRBTempConstr> violated_constraints;

  progressbar bar(degree_);
  for (int i = 0; i < degree_; ++i) {
    if (found >= config_.maxcut_) {
      break;
    }

    // only show progress bar in debugging mode (not in verbose because the PLOGV in the loop breaks the bar)
    if (plog::get()->getMaxSeverity() == plog::Severity::debug) {
      bar.update();
    }
    for (int j = 0; j < degree_; ++j) {
      if (i == j) continue;
      if (found >= config_.maxcut_) {
        break;
      }

      AuxGraph::Node start{false, false, i, j};
      AuxGraph::Node target{true, false, i, j};

      auto [cost, path] = aux->shortestPath(start, target);

      if (cost < min_path_cost) {
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
            switch (config_.inequality_type_) {

              case CircleInequality::TWO_CHORDED: {
                constraint_lhs += vars[EdgeIndex(node1.i, node1.j)];
              }
                break;
              case CircleInequality::HALF_CHORDED: {
                constraint_lhs -= vars[EdgeIndex(node1.i, node1.j)];
              }
                break;
            }
          }
            // trans-gadget edge, i.e. a negative edge in the violated constraint
            // because the auxiliary graph was build by adding (i,j) and (j,k),
            // we now know that this corresponds to the edge (i,k) in the original graph
          else if (node1.j == node2.i) {
            switch (config_.inequality_type_) {

              case CircleInequality::TWO_CHORDED: {
                constraint_lhs -= vars[EdgeIndex(node1.i, node2.j)];
              }
                break;
              case CircleInequality::HALF_CHORDED: {
                constraint_lhs += vars[EdgeIndex(node1.i, node2.j)];
              }
                break;
            }
          }
        }

        auto rhs = constraint_rhs(k);

        violated_constraints.emplace_back(constraint_lhs <= rhs);
        found++;

        PLOGV << "Found violated " << config_.inequality_type_ << " odd cycle inequality: " << constraint_lhs << "<= "
              << rhs << ".";

      }
    }
  }
  PLOGD << "Found " << found << " violated " << config_.inequality_type_ << " odd cycle-inequalities.";
  return violated_constraints;
}