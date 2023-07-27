//
// Created by max on 14.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_

enum class ST_Separator_Variant{
  GW1,
  GW2
};

#include "triangle_separator.h"
class ST_Separator : public AbstractSeparator {
 protected:
  const int maxcut_;
  const ST_Separator_Variant variant_;
 public:
  explicit ST_Separator(ModelWrapper &model, double tolerance, const int maxcut, ST_Separator_Variant variant)
	: AbstractSeparator(model, tolerance), maxcut_(maxcut), variant_(variant) {};

  int add_Cuts() override;
};

#endif //CLIQPART_SRC_SEPARATORS_ST_SEPARATOR_H_
