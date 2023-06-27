//
// Created by max on 09.06.23.
//

#include "run_config.h"
RunConfig RunConfig::FromFile(const string &path) {
  toml::table tbl;

  try {
	tbl = toml::parse_file(path);

	RunConfig config;

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

	// TODO actually read the used separator from the config file

	return config;
  } catch (const toml::parse_error &err) {
	PLOGF << "Could not parse run_config.toml. Error was: " << endl << err << endl;
  }
}
