//
// Created by max on 26.09.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_CIRCLESEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_CIRCLESEPARATOR_H_

#include "../abstract_separator.h"
#include "../../clique_part_model.h"

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
  const CircleInequality inequality_type_{CircleInequality::TWO_CHORDED};

  CircleSeparatorConfig() = default;

  explicit CircleSeparatorConfig(const CircleInequality ineq_type) : inequality_type_(ineq_type) {};

  CircleSeparatorConfig(double tolerance, const int maxcut, const CircleInequality ineq_type) : AbstractSeparatorConfig(
      tolerance,
      maxcut), inequality_type_(ineq_type) {};
};

class CircleSeparator : public AbstractSeparator<CircleSeparatorConfig> {
 public:
  CircleSeparator(int degree, const CircleSeparatorConfig &config) : AbstractSeparator(degree, config) {}
  vector<GRBTempConstr> SeparateSolution(double *solution, GRBVar *vars) override;

};

#endif //CLIQPART_SRC_SEPARATORS_CIRCLESEPARATOR_H_
