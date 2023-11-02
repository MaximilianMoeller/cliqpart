//
// Created by max on 07.08.23.
//

#include "ilp_callback.h"
#include "separators/triangle_separator.h"
#include "separators/st_separator.h"
ILPCallback::ILPCallback(CliquePartModel &model) : model(model) {
  // In the ILP Callback the triangle separator is only called when an integral vector has to be checked for feasibility
  // hence the tolerance can be set to 0.5.
  triangle_separator_ = make_unique<TriangleSeparator>(model.NodeCount(), TriangleSeparatorConfig(0.5, -1, false));
  //lp_separators_.emplace_back(make_unique<StSeparator>(model.NodeCount(), StSeparatorConfig()));
}

void ILPCallback::callback() {

  if (where != GRB_CB_MIPNODE && where != GRB_CB_MIPSOL) return;
  double *solution;
  if (where == GRB_CB_MIPSOL) {
	solution = getSolution(model.GetVars(), model.EdgeCount());
	auto lazy_constraints = triangle_separator_->SeparateSolution(solution, model.GetVars());
	for (const auto &constraint : lazy_constraints) {
	  addLazy(constraint);
	}
  } else if (where == GRB_CB_MIPNODE && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
	solution = getNodeRel(model.GetVars(), model.EdgeCount());

	// enumerating violated constraints
	vector<GRBTempConstr> violated_constraints = {};
	for (auto &separator : lp_separators_) {
	  violated_constraints = separator->SeparateSolution(solution, model.GetVars());
	  if (!violated_constraints.empty()) break;
	}
	for (const auto &constraint : violated_constraints) {
	  addCut(constraint);
	}
  }
}
