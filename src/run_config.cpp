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

	RunConfig config{};

	config.name = tbl["name"].value_or("");

	if (tbl.contains("run_count") && tbl["run_count"].is_integer()){
	  config.run_count = tbl["run_count"].value_or(1);
	}

	// row and column headers
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

	// degree of the graph
	if (!tbl.contains("run_settings") || !tbl["run_settings"].is_table()
	  || !tbl["run_settings"].as_table()->contains("graph_degree")
	  || !tbl["run_settings"]["graph_degree"].is_integer()) {
	  PLOGF << path << " must contain a table 'run_settings' with an integer value for the key 'graph_degree'!";
	  exit(-1);
	}
	config.graph_degree = tbl["run_settings"]["graph_degree"].value_or(-1);
	PLOGD << "Graph degree set to " << config.graph_degree;

	// offset for edge costs
	if (!tbl["run_settings"].as_table()->contains("value_offset")) {
	  PLOGI << path << ": no value offset was given, assuming 0."
			<< "If you’d like to subtract a constant offset from all values in the CSV table, please set it under 'run_settings.value_offset' in "
			<< path;
	}
	config.value_offset = tbl["run_settings"]["value_offset"].value_or(0.0);
	PLOGD << "Values in CSV table will be offset by " << config.value_offset;

	// tolerance to integrality
	if (!tbl["run_settings"].as_table()->contains("integrality_tolerance")) {
	  PLOGI << path << ": no integrality tolerance was given, assuming 1e-5.";
	}
	config.integrality_tolerance = tbl["run_settings"]["integrality_tolerance"].value_or(1e-5);
	PLOGD << "Solutions will be treated as integral with tolerance " << config.integrality_tolerance;

	// separators
	if (!tbl.contains("separators") || !tbl["separators"].is_array_of_tables()) {
	  PLOGF << path << " must contain an array of tables called 'separators'!";
	  exit(-1);
	}

	vector<variants> separators{};

	toml::array seps = *tbl["separators"].as_array();
	PLOGD << path << " contains " << seps.size() << " entries in the 'separators' array.";
	seps.for_each([&path, &config](toml::table &elem) mutable {
	  if (!elem.contains("inequality") || !elem["inequality"].is_string()) {

		PLOGF << path
			  << ": each table of the 'separators' array must contain a 'inequality' key with a value from the following list: "
			  << endl
			  << "Valid for triangle inequalities: ['Δ', 'Triangle', 'triangle', 'T', 't']" << endl
			  << "Valid for [S:T]-inequalities: ['st', 'ST', 's:t', 'S:T']" << endl
			  << "Skipping the current separator because these conditions were not met.";
		return;
	  }
	  auto ineq = elem["inequality"];
	  if (ineq == "Δ" || ineq == "Triangle" || ineq == "triangle" || ineq == "T" || ineq == "t") {
		if (!elem.contains("maxcut") || !elem["maxcut"].is_integer())
		  PLOGW << "Consider setting the 'maxcut' parameter when using a separator for Δ inequalities!"
				<< "Defaulting to -1, i.e. no limit on the number of inequalities added per iteration.";
		TriangleSeparatorConfig separator{elem["maxcut"].value_or(-1)};
		config.separators.emplace_back(separator);
	  } else if (ineq == "st" || ineq == "ST" || ineq == "s:t" || ineq == "S:T") {
		if (!elem.contains("maxcut") || !elem["maxcut"].is_integer())
		  PLOGW << "Consider setting the 'maxcut' parameter when using a separator for [S:T] inequalities!"
				<< "Defaulting to -1, i.e. no limit on the number of inequalities added per iteration.";
		ST_SeparatorConfig separator{elem["maxcut"].value_or(-1)};
		config.separators.emplace_back(separator);
	  } else {
		PLOGW << path << " contains a table in the 'separators' entry with the 'inequality' attribute specified to "
			  << ineq.as_string()
			  << " but this inequality is not supported (yet).";
	  }

	});
	if (seps.size() != config.separators.size()) {
	  PLOGW << "Only" << separators.size() << " of " << seps.size()
			<< " entries of the 'separators' array could be parsed into separators."
			<< "Please make sure to use TOMLs array of table syntax (see template) for the separators." << endl
			<< "Non-table entries are skipped during parsing.";
	}


	return config;
  } catch (const toml::parse_error &err) {
	PLOGF << "Could not parse run_config.toml. Error was: " << endl << err << endl;
  }
}
