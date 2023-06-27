//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_COMPLETE_GRAPH_H_
#define CLIQPART_SRC_COMPLETE_GRAPH_H_

#include <vector>
#include "gurobi_c++.h"
#include "run_config.h"

using namespace std;

class CompleteGraph {
 private:
  // the graph_degree of the complete graph, i.e. the number of nodes
  int degree_;

  // edge weights and the actual variables will be stored in continuously allocated memory.
  double *weights_;
  GRBVar *vars_;

 public:
  // needs information from the run config to generate edge weights,
  // a path to the CSV data_path,
  // and the gurobi-model to create the variables
  explicit CompleteGraph(RunConfig &config, const string &data_path, GRBModel &model);

  [[nodiscard]] int GetDegree() const { return degree_; };
  // often convenient, as the model will have a variable for every edge, not node.
  [[nodiscard]] int EdgeCount() const { return (degree_ * (degree_ - 1)) / 2; };

  // common error handling for `GetWeight` and `GetVar`, transforming logical, 2D indices into the
  // index for the continuously allocated internal representation
  [[nodiscard]] int GetIndex(int v1, int v2) const;

  // returns the objective weight of a given edge (indices of the adjacent nodes)
  double GetWeight(int v1, int v2) { return weights_[GetIndex(v1, v2)]; };
  // returns the gurobi variable object of a given edge (indices of the adjacent nodes)
  GRBVar GetVar(int v1, int v2) { return vars_[GetIndex(v1, v2)]; };
  GRBVar *GetVars() { return vars_; };

  ~CompleteGraph();
};

#endif //CLIQPART_SRC_COMPLETE_GRAPH_H_
