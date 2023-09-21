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
  // TODO move opening the file to the main function -> constructor adjustment

  rapidcsv::Document doc(data_path, rapidcsv::LabelParams(config.column_labels - 1, config.row_labels - 1));

  // iterates through the indices of the lower triangular matrix
  for (int row = 0; row < degree_ - 1; row++) {
	for (int column = row + 1; column < degree_; column++) {
	  // min for asymmetric data
	  auto obj_coefficient =
		  (min(doc.GetCell<double>(row, column), doc.GetCell<double>(column, row)) - config.value_offset)
			  * config.value_scaling;

	  string var_name{"x"};
	  // naming the variables according to the labels in the data.csv can facilitate debugging.
	  if (config.column_labels > 0 && config.row_labels > 0) {
		var_name +=
			"_" + doc.GetCell<string>(-1, row) + "_" + doc.GetCell<string>(column, -1);
	  } else {
		var_name += to_string(row) + "_" + to_string(column);
	  }
	  PLOGV << var_name << ": " << to_string(column * (column - 1) / 2 + row);

	  auto var = addVar(0.0,
						1.0,
						obj_coefficient,
						continuous ? GRB_CONTINUOUS : GRB_BINARY,
						var_name);
	  vars_[column * (column - 1) / 2 + row] = var;
	}
  }

  // model sense is determined by the data.toml
  set(GRB_IntAttr_ModelSense, config.maximizing ? -1 : 1);
}
