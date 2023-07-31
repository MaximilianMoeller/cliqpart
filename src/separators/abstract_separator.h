//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_

#include "../model_wrapper.h"

using namespace std;

class IAbstractSeparator {
 public:
  virtual int AddCuts() = 0;
};

class AbstractSeparatorConfig {
 public:
  double tolerance_{1e-6};
  explicit AbstractSeparatorConfig(double tolerance) : tolerance_(tolerance) {};
};

template<typename SeparatorConfig, typename = enable_if_t<is_base_of_v<AbstractSeparatorConfig, SeparatorConfig>>>
class AbstractSeparator : public IAbstractSeparator {
 protected:
  ModelWrapper &model_;
  SeparatorConfig &config_;
 public:
  explicit AbstractSeparator(ModelWrapper &model, SeparatorConfig &config)
	  : model_(model), config_(config) {};
};

#endif // CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
