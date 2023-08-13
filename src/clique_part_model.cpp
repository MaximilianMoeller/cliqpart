//
// Created by max on 09.06.23.
//

#include <iostream>
#include "rapidcsv.h"

#include "clique_part_model.h"

using namespace std;
CliquePartModel::CliquePartModel(GRBEnv &grb_env, const string &data_path, DataConfig config, bool continuous) :
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
						continuous ? GRB_CONTINUOUS : GRB_BINARY,
						"x_" + to_string(i) + "_" + to_string(j));
	  vars_[i * (i - 1) / 2 + j] = var;
	}
  }

}
