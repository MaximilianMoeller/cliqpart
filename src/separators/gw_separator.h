//
// Created by max on 14.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_GW_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_GW_SEPARATOR_H_

#include "cubic_triangle_separator.h"
class GWSeparator : public CubicTriangleSeparator {
 protected:
  const int maxcut_;
  void my_callback() override;
 public:
  explicit GWSeparator(CompleteGraph &graph, int maxcut) : CubicTriangleSeparator(graph), maxcut_(maxcut) {

  }
};

#endif //CLIQPART_SRC_SEPARATORS_GW_SEPARATOR_H_
