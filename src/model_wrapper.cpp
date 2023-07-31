//
// Created by max on 09.06.23.
//

#include <iostream>
#include "rapidcsv.h"

#include "model_wrapper.h"

using namespace std;
ModelWrapper::ModelWrapper(GRBEnv &grb_env, const string &data_path, DataConfig config) :
  GRBModel(grb_env),
  degree_(config.graph_degree),
  vars_(make_unique<GRBVar[]>(EdgeCount())) {
  // TODO add error handling (file not found, wrong value types, too few lines/columns, empty lines/columns, ignore cells (i,i), …)
  // TODO add configuration support (row/column headers (i.e. labels), global objective offset)
  // TODO move opening the file to the main function -> constructor adjustment
  rapidcsv::Document doc(data_path, rapidcsv::LabelParams(config.column_labels - 1, config.row_labels - 1));

  for (int i = 1; i < degree_; i++) {
	for (int j = 0; j < i; j++) {
	  auto obj_coefficient = (doc.GetCell<double>(i, j) - config.value_offset) * config.value_scaling;
	  auto var = addVar(0.0,
						1.0,
						obj_coefficient,
						GRB_CONTINUOUS,
						"x_" + to_string(i) + "_" + to_string(j));
	  vars_[GetIndex(i,j)] = var;
	}
  }

}
int ModelWrapper::GetIndex(int v1, int v2) const {
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
