//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_RUN_CONFIG_H_
#define CLIQPART_SRC_RUN_CONFIG_H_

#include <string>
#include <toml++/toml.h>
#include <plog/Log.h>

using namespace std;

// TODO read runconfig from config file

struct RunConfig {
  const int degree;
  double obj_offset;

  static RunConfig FromFile(const string& path);
};

#endif // CLIQPART_SRC_RUN_CONFIG_H_
