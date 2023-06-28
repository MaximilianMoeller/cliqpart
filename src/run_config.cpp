//
// Created by max on 09.06.23.
//

#include <plog/Log.h>
#include "run_config.h"
#include <string>
#include <toml++/toml.h>

RunConfig RunConfig::FromFile(const string &path) {
  toml::table tbl;

  try {
	tbl = toml::parse_file(path);

	// cannot create a struct with empty variant<…>
	RunConfig config{.separator = CubicSeparatorConfig{}};

	// TODO: (optional) use the field "name" from the config somehow

	if (!tbl.contains("data_file") || !tbl["data_file"].is_table()
	  || !tbl["data_file"].as_table()->contains("row_labels")
	  || !tbl["data_file"].as_table()->contains("column_labels")) {
	  PLOGW << "Please consider configuring the 'row_labels' and 'column_labels' values in the 'data_file' table in "
			<< path << "! " << "Missing values will be assumed 0, i.e. no row or column headers exist.";
	} else if (!tbl["data_file"]["row_labels"].is_integer() || !tbl["data_file"]["row_labels"].is_integer()) {
	  PLOGW << "Fields 'row_labels' and 'column_labels' must be integer! Assuming 0.";
	}

	config.column_labels = tbl["data_file"]["column_labels"].value_or(0);
	config.row_labels = tbl["data_file"]["row_labels"].value_or(0);
	PLOGD << "CSV data is assumed to have " << config.column_labels << " row(s) of column labels and "
		  << config.row_labels << " column(s) of row labels.";

	if (!tbl.contains("run_settings") || !tbl["run_settings"].is_table()
	  || !tbl["run_settings"].as_table()->contains("graph_degree")
	  || !tbl["run_settings"]["graph_degree"].is_integer()) {
	  PLOGF << path << " must contain a table 'run_settings' with an integer value for the key 'graph_degree'!";
	  exit(-1);
	}
	config.graph_degree = tbl["run_settings"]["graph_degree"].value_or(-1);
	PLOGD << "Graph degree set to " << config.graph_degree;

	if (!tbl["run_settings"].as_table()->contains("value_offset")) {
	  PLOGI << path << ": no value offset was given, assuming 0."
			<< "If you’d like to subtract a constant offset from all values in the CSV table, please set it under 'run_settings.value_offset' in "
			<< path;
	}
	config.value_offset = tbl["run_settings"]["value_offset"].value_or(0.0);
	PLOGD << "Values in CSV table will be offset by " << config.value_offset;

	if (!tbl.contains("separator") || !tbl["separator"].is_table()
	  || !tbl["separator"].as_table()->contains("type") || !tbl["separator"]["type"].is_string()) {
	  PLOGF << path << " must contain a table 'separator' with a string value for the key 'type'!";
	  exit(-1);
	}
	auto type = tbl["separator"]["type"];
	if (type == "gw" ||
	  type == "GW" ||
	  type == "GrötschelWakabayashi" ||
	  type == "GroetschelWakabayashi"
	  ) {
	  if (!tbl["separator"].as_table()->contains("maxcut") ||
		!tbl["separator"]["maxcut"].is_integer()) {
		PLOGW << "Consider setting the 'maxcut' parameter when using a GrötschelWakabayashi Separator!";
	  }
	  GWSeparatorConfig run_config{tbl["separator"]["maxcut"].value_or(400)};
	  config.separator.emplace<GWSeparatorConfig>(run_config);
	} else if (type == "ct" ||
	  type == "CT" ||
	  type == "CubicTriangle") {
	  CubicSeparatorConfig run_config;
	  config.separator.emplace<CubicSeparatorConfig>(run_config);
	} else {
	  PLOGF << path << " must contain a table 'separator' with a 'type' from the following list: " << endl
			<< "Valid for Grötschel-Wakabayashi-Separator: ['gw', 'GW', 'GrötschelWakabayashi', 'GroetschelWakabayashi']"
			<< endl
			<< "Valid for the cubic Triangle Separator: ['ct', 'CT', 'CubicTriangle']";
	  exit(-1);
	}

	return config;
  } catch (const toml::parse_error &err) {
	PLOGF << "Could not parse run_config.toml. Error was: " << endl << err << endl;
  }
}
