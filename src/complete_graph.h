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
  // common error handling of `get_weight` and `get_var`
  pair<double, GRBVar> get(int v1, int v2);

 public:
  // needs information from the run config to generate edge weights
  // and the gurobi-model to create the variables
  explicit CompleteGraph(RunConfig &config, GRBModel &model);

  [[nodiscard]] int get_degree() const { return degree_; };
  // often convenient, as the model will have a variable for every edge, not node.
  [[nodiscard]] int edge_count() const { return (degree_ * (degree_ - 1)) / 2; };

  // returns the objective weight of a given edge (indices of the adjacent nodes)
  double get_weight(int v1, int v2) { return get(v1, v2).first; };
  // returns the gurobi variable object of a given edge (indices of the adjacent nodes)
  GRBVar get_var(int v1, int v2) { return get(v1, v2).second; };
};

#endif //CLIQPART_SRC_COMPLETE_GRAPH_H_
