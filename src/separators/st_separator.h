//
// Created by max on 14.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_

#include "abstract_separator.h"
#include "../model_wrapper.h"

enum class StSeparatorHeuristic {
  GW1, GW2
};

class StSeparatorConfig : public AbstractSeparatorConfig {
 public:
  const int maxcut_{-1};
  const StSeparatorHeuristic heuristic_{StSeparatorHeuristic::GW1};
  StSeparatorConfig(double tolerance, const int maxcut, const StSeparatorHeuristic heuristic)
	  : AbstractSeparatorConfig(tolerance), maxcut_(maxcut), heuristic_(heuristic) {};
  StSeparatorConfig() = default;

};

#include "triangle_separator.h"
class StSeparator : public AbstractSeparator<StSeparatorConfig> {
 public:
  explicit StSeparator(const int degree, const StSeparatorConfig &config) : AbstractSeparator(degree, config) {};
  vector<GRBTempConstr> AddCuts(double *solution, GRBVar *vars) override;
};

#endif //CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
