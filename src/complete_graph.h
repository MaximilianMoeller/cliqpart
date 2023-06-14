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
// the degree of the complete graph, i.e. the number of nodes
  int degree_;
  // 2D-vector (degree x degree) of edge weights (double) and the corresponding gurobi-variables
  vector<vector<pair<double, GRBVar>>> edges_;
  // common error handling of `GetWeight` and `GetVar`
  pair<double, GRBVar> Get(int v1, int v2);

 public:
  // needs information from the run config to generate edge weights,
  // a path to the CSV data_path,
  // and the gurobi-model to create the variables
  explicit CompleteGraph(RunConfig &config, string data_path, GRBModel &model);

  [[nodiscard]] int GetDegree() const { return degree_; };
  // often convenient, as the model will have a variable for every edge, not node.
  [[nodiscard]] int EdgeCount() const { return (degree_ * (degree_ - 1)) / 2; };

  // returns the objective weight of a given edge (indices of the adjacent nodes)
  double GetWeight(int v1, int v2) { return Get(v1, v2).first; };
  // returns the gurobi variable object of a given edge (indices of the adjacent nodes)
  GRBVar GetVar(int v1, int v2) { return Get(v1, v2).second; };
};

#endif //CLIQPART_SRC_COMPLETE_GRAPH_H_
