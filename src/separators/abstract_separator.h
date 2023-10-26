//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_

#include "../clique_part_model.h"

using namespace std;

class IAbstractSeparator {
 protected:
  const int degree_;
  [[nodiscard]] int EdgeCount() const { return (degree_ * (degree_ - 1)) / 2; };
  [[nodiscard]] int EdgeIndex(int v1, int v2) const {
    if (v1 < 0 || v2 < 0 || v1 >= degree_ || v2 >= degree_) {
      PLOGF << "Access to variable v_" << v1 << "_" << v2
            << " was requested, but graph only has vertices from index 0 to " << degree_ - 1 << "!"
            << "This is most likely a mistake in the program logic of a separator.";
      exit(-1);
    }
    if (v1 == v2) {
      PLOGF << "Access to variable v_" << v1 << "_" << v2
            << " was requested, but graph does not contain reflexive edges."
            << "This is most likely a mistake in the program logic of a separator.";
      exit(-1);
    }

    if (v1 < v2) {
      return v2 * (v2 - 1) / 2 + v1;
    }
    return v1 * (v1 - 1) / 2 + v2;
  };

 public:
  explicit IAbstractSeparator(int degree) : degree_(degree) {};
  virtual ~IAbstractSeparator() = default;
  virtual vector<GRBTempConstr> SeparateSolution(double *solution, GRBVar *vars) = 0;
  virtual string Abbreviation() = 0;
};

class AbstractSeparatorConfig {
 public:
  const double tolerance_{1e-9};
  const int maxcut_{-1};
  AbstractSeparatorConfig(const double tolerance, const int maxcut) : tolerance_(tolerance), maxcut_(maxcut) {};
  AbstractSeparatorConfig() = default;
  virtual ~AbstractSeparatorConfig() = default;
};

template<typename SeparatorConfig, typename = enable_if_t<is_base_of_v<AbstractSeparatorConfig, SeparatorConfig>>>
class AbstractSeparator : public IAbstractSeparator {
 protected:
  const SeparatorConfig config_;
 public:
  explicit AbstractSeparator(int degree, const SeparatorConfig &config)
      : IAbstractSeparator(degree), config_(config) {};
};

#endif // CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
