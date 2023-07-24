//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_

#include <gurobi_c++.h>
#include "../model_wrapper.h"
#include "abstract_separator.h"

using namespace std;

class TriangleSeparator : public AbstractSeparator {
 private:
  const int maxcut_;
  const int node_count_;
 public:
  explicit TriangleSeparator(ModelWrapper &model, double tolerance, const int maxcut)
	: AbstractSeparator(model, tolerance), maxcut_(maxcut), node_count_(model.NodeCount()) {};
  ~TriangleSeparator() override = default;

 protected:
  int add_Cuts() override;
};

#endif // CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
