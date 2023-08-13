//
// Created by max on 07.08.23.
//

#include "ilp_callback.h"
#include "separators/triangle_separator.h"
ILPCallback::ILPCallback(CliquePartModel &model) : model(model) {
  triangle_separator_ = make_unique<TriangleSeparator>(model.NodeCount(), TriangleSeparatorConfig());
}

void ILPCallback::callback() {

  if (where != GRB_CB_MIPNODE && where != GRB_CB_MIPSOL) return;
  double *solution;
  if (where == GRB_CB_MIPSOL) {
	solution = getSolution(model.GetVars(), model.EdgeCount());
	auto lazy_Constraints = triangle_separator_->SeparateSolution(solution, model.GetVars());
	for (const auto &constraint : lazy_Constraints) {
	  addLazy(constraint);
	}
  } else if (where == GRB_CB_MIPNODE && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
	solution = getNodeRel(model.GetVars(), model.EdgeCount());
  }

}
