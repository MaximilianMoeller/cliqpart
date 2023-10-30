//
// Created by max on 26.09.23.
//

#include "CircleSeparator.h"
#include "AuxGraph.h"
#include "progressbar.hpp"

#define CSV_LOG 1

vector<GRBTempConstr> CircleSeparator::SeparateSolution(double *solution, GRBVar *vars) {
  PLOGD << "Starting search for violated " << config_.inequality_type_ << " odd cycle inequalities.";

  auto start_time = std::chrono::steady_clock::now();

  // what kind of auxiliary graph is used
  unique_ptr<AuxGraph> aux{};

  // how the weights in the auxiliary graph are set
  std::function<double(int, int)> gadget_weight;
  std::function<double(int, int)> triple_weight;


  // the minimum length of a path in the auxiliary graph such that the corresponding inequality is not violated
  double min_path_cost;
  // how the rhs of the constraint should be build depending on the circle length
  std::function<int(int)> constraint_rhs;

  // switch only once and prepare everything such that the rest of the code is the same for
  // two-chorded and half-chorded odd cycle inequalities
  switch (config_.inequality_type_) {

    case CircleInequality::TWO_CHORDED: {
      aux = make_unique<TwoChordedAuxGraph>(degree_, config_.time_limit_);

      gadget_weight =
          [this, &solution](int a, int b) { return std::clamp(0.5 - solution[EdgeIndex(a, b)], -0.5, 0.5); };
      triple_weight = [this, &solution](int a, int b) { return std::clamp(solution[EdgeIndex(a, b)], 0.0, 1.0); };

      min_path_cost = 0.5;
      constraint_rhs = [](int k) { return (k - 1) / 2; };
    }
      break;
    case CircleInequality::HALF_CHORDED: {
      aux = make_unique<HalfChordedAuxGraph>(degree_);

      gadget_weight = [this, &solution](int a, int b) { return std::clamp(solution[EdgeIndex(a, b)], 0.0, 1.0); };
      triple_weight = [this, &solution](int a, int b) { return std::clamp(1 - solution[EdgeIndex(a, b)], 0.0, 1.0); };

      min_path_cost = 3;
      constraint_rhs = [](int k) { return k - 3; };
    }
      break;
  }

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

      try {
        auto [cost, path] = aux->shortestPath(start, target);

        // terminate separation early if configured
        if (config_.time_limit_ > 0) {
          auto now = std::chrono::steady_clock::now();
          auto elapsed = (std::chrono::duration_cast<std::chrono::seconds>(now - start_time)).count();
          if (elapsed > config_.time_limit_) {
            throw AuxGraph::OutOfTimeException();
          }
        }

        if (cost < min_path_cost) {
          double violation_degree{0};
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
                  violation_degree += solution[EdgeIndex(node1.i, node1.j)];
                }
                  break;
                case CircleInequality::HALF_CHORDED: {
                  constraint_lhs -= vars[EdgeIndex(node1.i, node1.j)];
                  violation_degree -= solution[EdgeIndex(node1.i, node1.j)];
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
                  violation_degree -= solution[EdgeIndex(node1.i, node2.j)];
                }
                  break;
                case CircleInequality::HALF_CHORDED: {
                  constraint_lhs += vars[EdgeIndex(node1.i, node2.j)];
                  violation_degree += solution[EdgeIndex(node1.i, node2.j)];
                }
                  break;
              }
            }
          }

          auto rhs = constraint_rhs(k);
          violation_degree -= rhs;

          /* only add triangles if they violate the corresponding inequality by more than the tolerance
          // prevents an issue where, due to floating point arithmetic, the same inequality would be added over and over again
          */
          if (violation_degree > config_.tolerance_) {
            violated_constraints.emplace_back(constraint_lhs <= rhs);
            found++;

            PLOGV << "Found violated " << config_.inequality_type_ << " odd cycle inequality: " << constraint_lhs
                  << "<= "
                  << rhs << ".";

          }

        }
      }
      catch (AuxGraph::OutOfTimeException &_my_exception) {
        PLOGI_(CSV_LOG) << "{\"exception\":\"Time limit hit separating "// NOLINT(*-raw-string-literal)
                        << config_.inequality_type_
                        << " odd cycle inequalities.\"";
        PLOGD << "Time limit hit separating " << config_.inequality_type_ << " odd cycle inequalities."
              << " Found " << violated_constraints.size() << " violated constraints in that time.";
        return violated_constraints;
      }
    }
  }
  PLOGD << "Found " << found << " violated " << config_.inequality_type_ << " odd cycle inequalities.";
  return violated_constraints;
}