//
// Created by max on 14.06.23.
//

#include "gw_separator.h"
void GWSeparator::my_callback() {
  // in integral solutions only triangle inequalities are separated
  if (where == GRB_CB_MIPSOL) CubicTriangleSeparator::my_callback();
  else if (where == GRB_CB_MIPNODE && getIntInfo(GRB_CB_MIPNODE_STATUS) == GRB_OPTIMAL) {
	// TODO
  }
};