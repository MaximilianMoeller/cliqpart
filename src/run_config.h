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

struct TriangleSeparatorConfig {
  const int MAXCUT;
};

struct ST_SeparatorConfig {
  const int MAXCUT;
};

typedef variant<TriangleSeparatorConfig, ST_SeparatorConfig> variants;

struct RunConfig {
  string name;
  // run settings
  int run_count {1};
  int graph_degree;
  double value_offset;
  double value_scaling;
  double tolerance;
  // data file settings
  int row_labels;
  int column_labels;
  // list of separators
  vector<variants> separators;

  static RunConfig FromFile(const string &path);
};

#endif // CLIQPART_SRC_RUN_CONFIG_H_
