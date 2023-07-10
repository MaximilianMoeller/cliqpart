//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_

#include <gurobi_c++.h>
#include "../complete_graph.h"
#include "abstract_separator.h"

using namespace std;

class TriangleSeparator : public AbstractSeparator {
 public:
  explicit TriangleSeparator(CompleteGraph &graph) : AbstractSeparator(graph) {};
  ~TriangleSeparator() override = default;

 protected:
  void my_callback() override;
};

#endif // CLIQPART_SRC_SEPARATORS_TRIANGLE_SEPARATOR_H_
