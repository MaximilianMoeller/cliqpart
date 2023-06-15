//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_CUBIC_TRIANGLE_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_CUBIC_TRIANGLE_SEPARATOR_H_

#include <gurobi_c++.h>
#include "../complete_graph.h"
#include "abstract_separator.h"

using namespace std;

class CubicTriangleSeparator : public AbstractSeparator {
 public:
  explicit CubicTriangleSeparator(CompleteGraph &graph) : AbstractSeparator(graph) {};
  ~CubicTriangleSeparator() override = default;

 protected:
  void my_callback() override;
};

#endif // CLIQPART_SRC_SEPARATORS_CUBIC_TRIANGLE_SEPARATOR_H_
