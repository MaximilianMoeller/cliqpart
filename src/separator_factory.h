//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATOR_FACTORY_H_
#define CLIQPART_SRC_SEPARATOR_FACTORY_H_

#include "separators/abstract_separator.h"
#include "run_config.h"
#include "complete_graph.h"

class SeparatorFactory {
 public:
  static AbstractSeparator BuildSeparator(RunConfig& config, CompleteGraph& data);
};

#endif //CLIQPART_SRC_SEPARATOR_FACTORY_H_
