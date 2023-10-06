//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_RUN_CONFIG_H_
#define CLIQPART_SRC_RUN_CONFIG_H_

#include <string>
#include <toml++/toml.h>
#include <plog/Log.h>
#include <variant>
#include "separators/triangle_separator.h"
#include "separators/st_separator.h"
#include "separators/oddSequences/CircleSeparator.h"

using namespace std;

typedef variant<TriangleSeparatorConfig, StSeparatorConfig, CircleSeparatorConfig> ConfigVariant;

struct RunConfig {
 private:
  double tolerance;
 public:
  string name;
  // run settings
  int run_count{1};
  vector<ConfigVariant> separator_configs;

  explicit RunConfig(string &run_config_file);
};

#endif // CLIQPART_SRC_RUN_CONFIG_H_
