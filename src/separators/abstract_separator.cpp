//
// Created by max on 15.06.23.
//

#include "abstract_separator.h"

void AbstractSeparator::callback() {
  // fetch current solution if it could be of interest for the current callback
  // and only when this call is even valid
  if (where == GRB_CB_MIPSOL) {
	current_solution_ = GRBCallback::getSolution(graph_.GetVars(), graph_.EdgeCount());
  } else if (where == GRB_CB_MIPNODE && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
	current_solution_ = GRBCallback::getNodeRel(graph_.GetVars(), graph_.EdgeCount());
  }
  // actual callback
  my_callback();
  // invalidate old solution
  current_solution_ = nullptr;
}
double AbstractSeparator::getSolution(int i, int j) {
  if (current_solution_) {
	return current_solution_[graph_.GetIndex(i, j)];
  } else {
	PLOGE << "You tried to access the current solution without fetching it first." << endl
		  << "This could mean that you're trying to access the solution in a callback-code where this is not possible."
		  << endl
		  << "Returning 0.0, but please fix this!";
	return 0.0;
  }
}
