//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_

#include "abstract_separator.h"
#include "../model_wrapper.h"

class TriangleSeparatorConfig : public AbstractSeparatorConfig {
 public:
  const int maxcut_;
  const bool variable_once_;
  TriangleSeparatorConfig(double tolerance, const int maxcut, const bool variable_once = false)
	  : AbstractSeparatorConfig(tolerance), maxcut_(maxcut), variable_once_(variable_once) {};
};

class TriangleSeparator : public AbstractSeparator<TriangleSeparatorConfig> {
 public:
  TriangleSeparator(ModelWrapper &model, const TriangleSeparatorConfig &config) : AbstractSeparator(model, config) {}

 protected:
  int AddCuts() override;
};

#endif // CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
