//
// Created by max on 26.09.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_CIRCLESEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_CIRCLESEPARATOR_H_

#include "../abstract_separator.h"
#include "../../clique_part_model.h"
#include "AuxGraph.h"

enum class CircleInequality {
  TWO_CHORDED, HALF_CHORDED
};

inline ostream &operator<<(ostream &out, CircleInequality c) {
  switch (c) {
    case CircleInequality::TWO_CHORDED: out << "2-chorded";
      break;
    case CircleInequality::HALF_CHORDED: out << "half-chorded";
      break;
  }
  return out;
}

class CircleSeparatorConfig : public AbstractSeparatorConfig {
 public:
  const int time_limit_{-1};
  const CircleInequality inequality_type_{CircleInequality::TWO_CHORDED};

  CircleSeparatorConfig() = default;

  explicit CircleSeparatorConfig(const CircleInequality ineq_type, const int time_limit)
      : inequality_type_(ineq_type), time_limit_(time_limit) {};

  CircleSeparatorConfig(double tolerance, const int maxcut, const CircleInequality ineq_type, const int time_limit)
      : AbstractSeparatorConfig(
      tolerance,
      maxcut), inequality_type_(ineq_type), time_limit_(time_limit) {};
};

class CircleSeparator : public AbstractSeparator<CircleSeparatorConfig> {
 public:
  CircleSeparator(int degree, const CircleSeparatorConfig &config) : AbstractSeparator(degree, config) {}
  vector<GRBTempConstr> SeparateSolution(double *solution, GRBVar *vars) override;
  string Abbreviation() override {
    string res;
    switch (config_.inequality_type_) {
      case CircleInequality::TWO_CHORDED:res = "two-chorded";
        break;
      case CircleInequality::HALF_CHORDED:res = "half-chorded";
        break;
    }
    return res;
  }
};

#endif //CLIQPART_SRC_SEPARATORS_CIRCLESEPARATOR_H_
