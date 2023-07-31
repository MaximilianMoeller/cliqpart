//
// Created by max on 09.06.23.
//

#include <plog/Log.h>
#include "run_config.h"
#include <string>
#include <toml++/toml.h>
#include <algorithm>
#include "separators/st_separator.h"

RunConfig::RunConfig(string &run_config_file) {
  toml::table tbl;

  try {
	tbl = toml::parse_file(run_config_file);

	if (!tbl.contains("name") || !tbl["name"].is_string()) {
	  PLOGW << "Please provide a string typed 'name' field in the config file " << run_config_file << "! ";
	}
	auto backup_name = run_config_file;
	std::string::iterator end_pos = std::remove(backup_name.begin(), backup_name.end(), '/');
	backup_name.erase(end_pos, backup_name.end());
	name = tbl["name"].value_or(backup_name);
	PLOGD << "Result of parsing " << run_config_file << " will be named " << name << ".";

	if (!tbl.contains("run_count") || !tbl["run_count"].is_integer()) {
	  PLOGI << "No 'run_count' key was given in " << run_config_file << ". Assuming 1.";
	}
	run_count = tbl["run_count"].value_or(1);
	PLOGD << "Run count of the configuration " << name << " set to " << run_count << ".";

	// tolerance to integrality
	if (!tbl.contains("tolerance")) {
	  PLOGI << run_config_file << ": no integrality tolerance was given, assuming 1e-6.";
	}
	tolerance = tbl["tolerance"].value_or(1e-6);
	PLOGD << "Solutions will be treated as integral with tolerance " << tolerance;

	// separators
	if (!tbl.contains("separators") || !tbl["separators"].is_array_of_tables()) {
	  PLOGF << run_config_file << " must contain an array of tables called 'separators'!";
	  exit(-1);
	}

	toml::array seps = *tbl["separators"].as_array();
	PLOGD << run_config_file << " contains " << seps.size() << " entries in the 'separators' array.";
	seps.for_each([&run_config_file, this](toml::table &elem) mutable {
	  if (!elem.contains("inequality") || !elem["inequality"].is_string()) {

		PLOGF << run_config_file
			  << ": each table of the 'separators' array must contain a 'inequality' key with a value from the following list: "
			  << endl << "Valid for triangle inequalities: ['Δ', 'Triangle', 'triangle', 'T', 't']" << endl
			  << "Valid for [S:T]-inequalities: ['st', 'ST', 's:t', 'S:T']" << endl
			  << "Skipping the current separator because these conditions were not met.";
		return;
	  }
	  auto ineq = elem["inequality"];
	  if (ineq == "Δ" || ineq == "Triangle" || ineq == "triangle" || ineq == "T" || ineq == "t") {
		if (!elem.contains("maxcut") || !elem["maxcut"].is_integer()) {
		  PLOGW << "Consider setting the 'maxcut' parameter when using a separator for Δ inequalities!"
				<< "Defaulting to -1, i.e. no limit on the number of inequalities added per iteration.";
		}
		separator_configs.emplace_back(TriangleSeparatorConfig{tolerance, elem["maxcut"].value_or(-1)});
	  } else if (ineq == "st" || ineq == "ST" || ineq == "s:t" || ineq == "S:T") {
		if (!elem.contains("maxcut") || !elem["maxcut"].is_integer()) {
		  PLOGW << "Consider setting the 'maxcut' parameter when using a separator for [S:T] inequalities!"
				<< "Defaulting to -1, i.e. no limit on the number of inequalities added per iteration.";
		}
		StSeparatorHeuristic heuristic;
		if (!elem.contains("heuristic") || !elem["heuristic"].is_integer()) {
		  PLOGW << "No heuristic was given for a [S:T]-separator in " << run_config_file
				<< ". Assuming a default one.";
		}
		int h = elem["heuristic"].value_or(0);
		switch (h) {
		  case 1: heuristic = StSeparatorHeuristic::GW1;
			break;
		  case 2: heuristic = StSeparatorHeuristic::GW2;
			break;
		  default: heuristic = StSeparatorHeuristic::GW1;
		}
		separator_configs.emplace_back(StSeparatorConfig{tolerance,
														 elem["maxcut"].value_or(-1),
														 heuristic});
	  } else {
		PLOGW << run_config_file
			  << " contains a table in the 'separators' entry with the 'inequality' attribute specified to "
			  << ineq.as_string() << " but this inequality is not supported (yet).";
	  }

	});
	if (seps.size() != separator_configs.size()) {
	  PLOGW << "Only" << separator_configs.size() << " of " << seps.size()
			<< " entries of the 'separators' array could be parsed into separators."
			<< "Please make sure to use TOMLs array of table syntax (see template) for the separators." << endl
			<< "Non-table entries are skipped during parsing.";
	}
  } catch (const toml::parse_error &err) {
	PLOGF << "Could not parse run_config.toml. Error was: " << endl << err << endl;
	exit(-1);
  }
}
