//
// Created by max on 07.08.23.
//

#ifndef CLIQPART_SRC_ILP_CALLBACK_H_
#define CLIQPART_SRC_ILP_CALLBACK_H_

#include <gurobi_c++.h>
#include "clique_part_model.h"
#include "separators/abstract_separator.h"
#include "separators/triangle_separator.h"

class ILPCallback : public GRBCallback {
 private:
  CliquePartModel& model;
  unique_ptr<TriangleSeparator> triangle_separator_;
  vector<unique_ptr<IAbstractSeparator>> lp_separators_;

 public:
  explicit ILPCallback(CliquePartModel &model);

  void callback() override;

};

#endif //CLIQPART_SRC_ILP_CALLBACK_H_
