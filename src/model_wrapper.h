//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_MODEL_WRAPPER_H_
#define CLIQPART_SRC_MODEL_WRAPPER_H_

#include <vector>
#include "gurobi_c++.h"
#include "run_config.h"

using namespace std;

class ModelWrapper : public GRBModel {
 private:
  // the graph_degree of the complete graph, i.e. the number of nodes
  int degree_;

  unique_ptr<GRBVar[]> vars_;
  [[nodiscard]] int GetIndex(int v1, int v2) const;

 public:
  // needs information from the run config to generate edge weights,
  // a path to the CSV data_path,
  // and the gurobi environment needed to create the model and its variables
  explicit ModelWrapper(GRBEnv &grb_env, RunConfig &config, const string &data_path);

  [[nodiscard]] int NodeCount() const { return degree_; };
  // often convenient, as the model will have a variable for every edge, not node.
  [[nodiscard]] int EdgeCount() const { return (degree_ * (degree_ - 1)) / 2; };

  // returns the current solution of a given edge (indices of the adjacent nodes)
  double GetSolution(int v1, int v2) { return GetVar(v1, v2).get(GRB_DoubleAttr_X); };

  // returns the objective weight of a given edge (indices of the adjacent nodes)
  double GetWeight(int v1, int v2) { return GetVar(v1, v2).get(GRB_DoubleAttr_Obj); };

  // returns the gurobi variable object of a given edge (indices of the adjacent nodes)
  GRBVar GetVar(int v1, int v2) { return vars_[GetIndex(v1, v2)]; };
  GRBVar *GetVars() { return vars_.get(); };
};

#endif //CLIQPART_SRC_MODEL_WRAPPER_H_
