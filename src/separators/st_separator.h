//
// Created by max on 14.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_

#include "triangle_separator.h"
class ST_Separator : public AbstractSeparator {
 protected:
  const int maxcut_;
  void my_callback() override;
 public:
  explicit ST_Separator(CompleteGraph &graph, int maxcut) : AbstractSeparator(graph), maxcut_(maxcut) {

  }
};

#endif //CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
