//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_

#include "abstract_separator.h"
#include "../model_wrapper.h"

class TriangleSeparatorConfig : public AbstractSeparatorConfig {
 public:
  const int maxcut_{-1};
  const bool variable_once_{false};
  TriangleSeparatorConfig(double tolerance, const int maxcut, const bool variable_once)
	  : AbstractSeparatorConfig(tolerance), maxcut_(maxcut), variable_once_(variable_once) {};
  TriangleSeparatorConfig() = default;
};

class TriangleSeparator : public AbstractSeparator<TriangleSeparatorConfig> {
 public:
  TriangleSeparator(int degree, const TriangleSeparatorConfig &config) : AbstractSeparator(degree, config) {}
  vector<GRBTempConstr> AddCuts(double *solution, GRBVar *vars) override;
};

#endif // CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
