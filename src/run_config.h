//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_RUN_CONFIG_H_
#define CLIQPART_SRC_RUN_CONFIG_H_

#include <string>
#include <toml++/toml.h>
#include <plog/Log.h>
#include <variant>

using namespace std;

struct GWSeparatorConfig {
  const int MAXCUT;
};

struct CubicSeparatorConfig {};

struct RunConfig {
  int row_labels;
  int column_labels;
  variant<GWSeparatorConfig, CubicSeparatorConfig> separator;
  int graph_degree;
  double value_offset;

  static RunConfig FromFile(const string &path);
};

#endif // CLIQPART_SRC_RUN_CONFIG_H_
