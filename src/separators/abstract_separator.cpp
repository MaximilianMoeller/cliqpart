//
// Created by max on 15.06.23.
//

#include "abstract_separator.h"

void AbstractSeparator::callback() {
  // fetch current solution if it could be of interest for the current callback
  // and only when this call is even valid
  if (where == GRB_CB_MIPSOL || (where == GRB_CB_MIPNODE && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL))
	FetchSolution();
  // actual callback
  my_callback();
  // invalidate old solution
  current_solution_ = nullptr;
}
void AbstractSeparator::FetchSolution() {
  current_solution_ = GRBCallback::getSolution(graph_.GetVars(), graph_.EdgeCount());
}
double AbstractSeparator::getSolution(int i, int j) {
  if (current_solution_) {
	return current_solution_[graph_.GetIndex(i, j)];
  } else {
	PLOGE << "You tried to access the current solution without fetching it first." << endl;
	exit(-1);
  }
}
