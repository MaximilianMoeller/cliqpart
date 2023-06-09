//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_CUBIC_TRIANGLE_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_CUBIC_TRIANGLE_SEPARATOR_H_

#include <gurobi_c++.h>

using namespace std;

class CubicTriangleSeparator : public GRBCallback {
 private:
  GRBVar **vars_;
  int degree_;

 public:
  CubicTriangleSeparator(GRBVar **xvars, int xn) : vars_(xvars), degree_(xn) {}

 protected:
  void callback() override;
};

#endif // CLIQPART_SRC_SEPARATORS_CUBIC_TRIANGLE_SEPARATOR_H_
