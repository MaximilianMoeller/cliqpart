//
// Created by max on 14.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_

#include "abstract_separator.h"
#include "../model_wrapper.h"

enum class StSeparatorHeuristic {
  GW1,
  GW2
};

class StSeparatorConfig : public AbstractSeparatorConfig {
 public:
  int maxcut_;
  StSeparatorHeuristic heuristic_;
  StSeparatorConfig(double tolerance, int maxcut, StSeparatorHeuristic heuristic)
	  : AbstractSeparatorConfig(tolerance), maxcut_(maxcut), heuristic_(heuristic) {};

};

#include "triangle_separator.h"
class StSeparator : public AbstractSeparator<StSeparatorConfig> {
 public:
  explicit StSeparator(ModelWrapper &model, StSeparatorConfig &config)
	  : AbstractSeparator(model, config) {};

  int AddCuts() override;
};

#endif //CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
