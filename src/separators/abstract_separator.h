//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_

#include "../model_wrapper.h"

using namespace std;

class IAbstractSeparator {
 public:
  virtual ~IAbstractSeparator() = default;
  virtual int AddCuts() = 0;
};

class AbstractSeparatorConfig {
 public:
  const double tolerance_{1e-6};
  explicit AbstractSeparatorConfig(const double tolerance) : tolerance_(tolerance) {};
  virtual ~AbstractSeparatorConfig() = default;
};

template<typename SeparatorConfig, typename = enable_if_t<is_base_of_v<AbstractSeparatorConfig, SeparatorConfig>>>
class AbstractSeparator : public IAbstractSeparator {
 protected:
  ModelWrapper &model_;
  // I thought this should be a 'const SeparatorConfig &config_',
  // but the runs just make noooo sense if it is, i.e. the maxcut member variable of a TriangleSeparatorConfig gets
  // changed even when constant.
  // I guess my C++ is not good enough to understand this.
  const SeparatorConfig config_;
 public:
  explicit AbstractSeparator(ModelWrapper &model, const SeparatorConfig &config) : model_(model), config_(config) {};
};

#endif // CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
