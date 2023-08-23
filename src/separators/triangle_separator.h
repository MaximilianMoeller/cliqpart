//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_

#include "abstract_separator.h"
#include "../clique_part_model.h"

class TriangleSeparatorConfig : public AbstractSeparatorConfig {
 public:
  const int maxcut_{-1};
  const bool variable_once_{false};
  TriangleSeparatorConfig(double tolerance, const int maxcut, const bool variable_once)
	  : AbstractSeparatorConfig(tolerance), maxcut_(maxcut), variable_once_(variable_once) {};
  TriangleSeparatorConfig() = default;
  TriangleSeparatorConfig(const int maxcut, const bool variable_once)
	  : maxcut_(maxcut), variable_once_(variable_once) {};
};

class TriangleSeparator : public AbstractSeparator<TriangleSeparatorConfig> {
 public:
  TriangleSeparator(int degree, const TriangleSeparatorConfig &config) : AbstractSeparator(degree, config) {}
  vector<GRBTempConstr> SeparateSolution(double *solution, GRBVar *vars) override;
};

#endif // CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
