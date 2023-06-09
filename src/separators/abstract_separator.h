//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_

#include "../verbosity.h"
#include <gurobi_c++.h>

using namespace std;

class AbstractSeparator : public GRBCallback {
 public:
  // Should only comment on errors by default.
  AbstractSeparator() : verbosity_(Verbosity::ERROR) {}
  ~AbstractSeparator() override = 0;

 protected:
  Verbosity verbosity_;
  void callback() override = 0;
};

#endif // CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
