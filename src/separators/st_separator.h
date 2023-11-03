//
// Created by max on 14.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_

#include "abstract_separator.h"
#include "../clique_part_model.h"

enum class StSeparatorHeuristic {
  GW1, GW2
};
inline ostream &operator<<(ostream &out, StSeparatorHeuristic h) {
  switch (h) {
    case StSeparatorHeuristic::GW1: out << "GW1";
      break;
    case StSeparatorHeuristic::GW2: out << "GW2";
      break;
  }
  return out;
}

class StSeparatorConfig : public AbstractSeparatorConfig {
 public:
  const StSeparatorHeuristic heuristic_{StSeparatorHeuristic::GW1};

  StSeparatorConfig() = default;

  explicit StSeparatorConfig(const StSeparatorHeuristic heuristic) : heuristic_(heuristic) {};

  StSeparatorConfig(double tolerance, const int maxcut, const StSeparatorHeuristic heuristic) : AbstractSeparatorConfig(
      tolerance, maxcut), heuristic_(heuristic) {};

};

class StSeparator : public AbstractSeparator<StSeparatorConfig> {
 public:
  explicit StSeparator(const int degree, const StSeparatorConfig &config) : AbstractSeparator(degree, config) {};
  vector<GRBTempConstr> SeparateSolution(double *solution, GRBVar *vars) override;
  string Abbreviation() override { return "[S:T]"; };
};

#endif //CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
