//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_RUN_CONFIG_H_
#define CLIQPART_SRC_RUN_CONFIG_H_

#include <string>
using namespace std;

// TODO read runconfig from config file

struct RunConfig {
  const int degree;
  const string graph_data;

  double obj_offset;
};

#endif // CLIQPART_SRC_RUN_CONFIG_H_
