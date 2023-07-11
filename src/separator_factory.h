//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATOR_FACTORY_H_
#define CLIQPART_SRC_SEPARATOR_FACTORY_H_

#include <memory>
#include "separators/abstract_separator.h"
#include "run_config.h"
#include "model_wrapper.h"

class SeparatorFactory {
 public:
  static vector<unique_ptr<AbstractSeparator>> BuildSeparator(RunConfig& config, ModelWrapper& data);
};

#endif //CLIQPART_SRC_SEPARATOR_FACTORY_H_
