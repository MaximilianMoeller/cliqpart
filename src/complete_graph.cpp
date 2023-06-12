//
// Created by max on 09.06.23.
//

#include <iostream>
#include <vector>
#include "rapidcsv.h"

#include "complete_graph.h"

using namespace std;
CompleteGraph::CompleteGraph(RunConfig &config, GRBModel &model) {
  degree_ = config.degree;

  // TODO add error handling (file not found, wrong value types, too few lines/columns, empty lines/columns, ignore cells (i,i), …)
  // TODO add configuration support (row/column headers (i.e. labels), global objective offset)
  rapidcsv::Document doc(config.graph_data, rapidcsv::LabelParams(0, 0));

  for (int i = 0; i < degree_; i++) {
	vector<pair<double, GRBVar>> row;
	for (int j = 0; j < i; j++) {
	  auto obj_coefficient = doc.GetCell<double>(i, j) - config.obj_offset;
	  auto var = model.addVar(0.0,
							  1.0,
							  obj_coefficient,
							  GRB_BINARY,
							  "x_" + to_string(i) + "_" + to_string(j));

	  row.emplace_back(obj_coefficient, var);
	}
	edges_.push_back(row);
  }

}
pair<double, GRBVar> CompleteGraph::Get(int v1, int v2) {
  if (v1 >= degree_ || v2 >= degree_) {
	clog << "Access to variable v_" << v1 << "_" << v2
		 << " was requested, but graph only has variables from index 0 to " << degree_ - 1 << endl;
  }

  if (v1 < v2) {
	return edges_[v2][v1];
  }
  return edges_[v1][v2];
}
