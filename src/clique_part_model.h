//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_CLIQUE_PART_MODEL_H_
#define CLIQPART_SRC_CLIQUE_PART_MODEL_H_

#include <vector>
#include "gurobi_c++.h"
#include "data.h"

using namespace std;

class CliquePartModel : public GRBModel {
 private:
  // the graph_degree of the complete graph, i.e. the number of nodes
  int degree_;
  double integrality_tolerance;

  unique_ptr<GRBVar[]> vars_;

 public:
  // needs information from the run config to generate edge weights,
  // a path to the CSV data_path,
  // and the gurobi environment needed to create the model and its variables
  explicit CliquePartModel(GRBEnv &grb_env, const string &data_path, DataConfig config, bool continuous = true);

  [[nodiscard]] int NodeCount() const { return degree_; };
  // often convenient, as the model will have a variable for every edge, not node.
  [[nodiscard]] int EdgeCount() const { return (degree_ * (degree_ - 1)) / 2; };

  // returns the current solution (only valid after LP-optimization finished)
  double *GetSolution() { return get(GRB_DoubleAttr_X, vars_.get(), EdgeCount()); }

  // whether the current solution is integral (only valid after LP-optimization finished)
  bool IsIntegral();

  // returns the gurobi variable object of a given edge (indices of the adjacent nodes)
  GRBVar *GetVars() { return vars_.get(); };
};

#endif //CLIQPART_SRC_CLIQUE_PART_MODEL_H_
