//
// Created by max on 14.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_

#include "triangle_separator.h"
class ST_Separator : public AbstractSeparator {
 protected:
  const int maxcut_;
 public:
  explicit ST_Separator(ModelWrapper &model, double tolerance, const int maxcut)
	: AbstractSeparator(model, tolerance), maxcut_(maxcut) {};

  int add_Cuts() override;
};

#endif //CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
