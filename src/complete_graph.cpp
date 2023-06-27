//
// Created by max on 09.06.23.
//

#include <iostream>
#include <vector>
#include "rapidcsv.h"

#include "complete_graph.h"

using namespace std;
CompleteGraph::CompleteGraph(RunConfig &config, const string &data_path, GRBModel &model) {
  degree_ = config.degree;
  weights_ = new double[EdgeCount()];
  vars_ = new GRBVar[EdgeCount()];

  // TODO add error handling (file not found, wrong value types, too few lines/columns, empty lines/columns, ignore cells (i,i), …)
  // TODO add configuration support (row/column headers (i.e. labels), global objective offset)
  // TODO move opening the file to the main function -> constructor adjustment
  rapidcsv::Document doc(data_path, rapidcsv::LabelParams(0, 0));

  for (int i = 1; i < degree_; i++) {
	for (int j = 0; j < i; j++) {
	  auto obj_coefficient = doc.GetCell<double>(i, j) - config.obj_offset;
	  auto var = model.addVar(0.0,
							  1.0,
							  obj_coefficient,
							  GRB_BINARY,
							  "x_" + to_string(i) + "_" + to_string(j));
	  weights_[i * (i - 1) / 2 + j] = obj_coefficient;
	  vars_[i * (i - 1) / 2 + j] = var;
	}
  }

}
int CompleteGraph::GetIndex(int v1, int v2) const {
  if (v1 < 0 || v2 < 0 || v1 >= degree_ || v2 >= degree_) {
	PLOGF << "Access to variable v_" << v1 << "_" << v2
		  << " was requested, but graph only has vertices from index 0 to " << degree_ - 1 << endl;
	exit(-1);
  }
  if (v1 == v2) {
	PLOGF << "Access to variable v_" << v1 << "_" << v2
		  << " was requested, but graph does not contain reflexive edges.";
	exit(-1);
  }

  if (v1 < v2) {
	return v2 * (v2 - 1) / 2 + v1;
  }
  return v1 * (v1 - 1) / 2 + v2;
}
CompleteGraph::~CompleteGraph() {
	delete[] vars_;
	delete[] weights_;
}
