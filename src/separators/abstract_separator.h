//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_

#include "../complete_graph.h"
#include <gurobi_c++.h>

using namespace std;

class AbstractSeparator : public GRBCallback {
 protected:
  explicit AbstractSeparator(CompleteGraph &graph) :  graph_(graph) {}
  CompleteGraph& graph_;
  void callback() override = 0;
};

#endif // CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
