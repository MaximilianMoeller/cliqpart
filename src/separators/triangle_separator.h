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
  TriangleSeparatorConfig(double tolerance, const int maxcut) : AbstractSeparatorConfig(tolerance), maxcut_(maxcut) {};
};

class TriangleSeparator : public AbstractSeparator<TriangleSeparatorConfig> {
 public:
  TriangleSeparator(ModelWrapper &model, const TriangleSeparatorConfig &config) : AbstractSeparator(model, config) {}

 protected:
  int AddCuts() override;
};

#endif // CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
