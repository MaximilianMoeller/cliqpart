//
// Created by max on 09.06.23.
//

#include <iostream>
#include "rapidcsv.h"

#include "clique_part_model.h"

using namespace std;
CliquePartModel::CliquePartModel(GRBEnv &grb_env, const string &data_path, DataConfig config, bool continuous) :
    GRBModel(grb_env),
    degree_(config.graph_degree),
    integrality_tolerance(config.integrality_tolerance),
    vars_(make_unique<GRBVar[]>(EdgeCount())) {
  // TODO add error handling (file not found, wrong value types, too few lines/columns, empty lines/columns, ignore cells (i,i), …)
  // TODO move opening the file to the main function -> constructor adjustment

  rapidcsv::Document doc(data_path, rapidcsv::LabelParams(config.column_labels - 1, config.row_labels - 1));

  // iterates through the indices of the lower triangular matrix
  for (int row = 0; row < degree_ - 1; row++) {
    for (int column = row + 1; column < degree_; column++) {
      // min for asymmetric data
      auto obj_coefficient =
          (min(doc.GetCell<double>(row, column), doc.GetCell<double>(column, row)) - config.value_offset)
              * config.value_scaling;

      string var_name{"x"};
      // naming the variables according to the labels in the data.csv can facilitate debugging.
      if (config.column_labels > 0 && config.row_labels > 0) {
        var_name +=
            "_" + doc.GetCell<string>(-1, row) + "_" + doc.GetCell<string>(column, -1);
      } else {
        var_name += to_string(row) + "_" + to_string(column);
      }
      PLOGV << "Variable '" << var_name << "' is placed at index " << to_string(column * (column - 1) / 2 + row)
            << " in continuous memory.";

      auto var = addVar(0.0,
                        1.0,
                        obj_coefficient,
                        continuous ? GRB_CONTINUOUS : GRB_BINARY,
                        var_name);
      vars_[column * (column - 1) / 2 + row] = var;
    }
  }

  // model sense is determined by the data.toml
  set(GRB_IntAttr_ModelSense, config.maximizing ? -1 : 1);
  // assure that any objective found in the first iteration is worse than the current one
  worst_objective_ =
      config.maximizing ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();
}
bool CliquePartModel::IsIntegral() {
  auto sol = GetSolution();
  std::vector<double> vec_sol(sol, sol + EdgeCount());
  return std::all_of(vec_sol.begin(),
                     vec_sol.end(),
                     [this](double x) { return std::abs(x - std::floor(x + 0.5)) <= integrality_tolerance; });
}

bool CliquePartModel::ObjectiveDeclined(double current_objective) {
  // maximizing = -1, minimizing = 1
  int sense = get(GRB_IntAttr_ModelSense);

  return (sense * worst_objective_ < sense * current_objective);
}

int CliquePartModel::DeleteCuts() {
  int deleted{0};
  double current_obj_value = get(GRB_DoubleAttr_ObjVal);

  // The algorithm can get stuck in a hyperplane for some time, and removing cuts while still in this hyperplane
  // would lead to an infinite loop.
  // Therefore, only consider removing constraints when the objective value declined,
  // i.e., the hyperplane got cut off entirely, and the LP relaxation now has a worse objective value than before.
  if (ObjectiveDeclined(current_obj_value)) {
    PLOGD << "Objective declined from " << worst_objective_ << " to " << current_obj_value << "! "
          << "Searching for inactive constraints to remove.";
    auto constraint_count = get(GRB_IntAttr_NumConstrs);
    auto constraints = getConstrs();

    for (int i = 0; i < constraint_count; ++i) {
      GRBConstr constraint = constraints[i];
      if (constraint.get(GRB_DoubleAttr_Slack) > 1e-9) {
        PLOGV << "Removing " << getRow(constraint) << constraint.get(GRB_CharAttr_Sense) << "= "
              << constraint.get(GRB_DoubleAttr_RHS) << ". Current LHS value was " << getRow(constraint).getValue()
              << ".";
        // Due to Gurobis lazy update approach, this change won't take effect until update, optimize or write is called
        // on the model.
        // Thus, this loop does not invalidate itself.
        // Thanks, Gurobi.
        remove(constraint);
        deleted++;
      }
    }

    worst_objective_ = current_obj_value;
  } else {
    PLOGD << "Objective did not decline, is still " << current_obj_value << "! Not deleting any cuts.";
  }

  return deleted;
}
