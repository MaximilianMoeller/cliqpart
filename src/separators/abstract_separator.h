//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_

#include "../model_wrapper.h"
#include <gurobi_c++.h>

using namespace std;

class AbstractSeparator {
 protected:
  ModelWrapper &model_;
  double tolerance;
  int node_count_;
  int edge_count_;
 public:
  explicit AbstractSeparator(ModelWrapper &model, double precision)
	: model_(model), tolerance(precision), node_count_(model_.NodeCount()), edge_count_(model_.EdgeCount()) {};
  virtual int add_Cuts() = 0;

  virtual ~AbstractSeparator() = default;
};

#endif // CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
