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
      PLOGW << "Please provide a string typed field called 'name' in the config file " << run_config_file << "! ";
    }
    auto backup_name = filesystem::path(run_config_file).filename().string();
    std::string::iterator end_pos = std::remove(backup_name.begin(), backup_name.end(), '/');
    backup_name.erase(end_pos, backup_name.end());
    name = tbl["name"].value_or(backup_name);
    PLOGD << run_config_file << ": Parsing result will be named " << name << ".";

    if (!tbl.contains("run_count") || !tbl["run_count"].is_integer()) {
      PLOGW << run_config_file << ": No 'run_count' key was given. Assuming 1. "
            << "If you wish re-running this config multiple times, set the 'run_count' parameter in "
            << run_config_file << ".";
    }
    run_count = tbl["run_count"].value_or(1);
    PLOGD << name << " will be run " << run_count << " time(s).";

    // tolerance to integrality
    if (!tbl.contains("tolerance")) {
      PLOGW << run_config_file << ": No integrality tolerance was given, assuming 1e-6.";
    }
    tolerance = tbl["tolerance"].value_or(1e-6);
    PLOGD << name << ": Solutions will be treated as integral with tolerance " << tolerance;

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
              << "Valid for triangle inequalities: ['Δ', 'Triangle', 'triangle', 'T', 't']. "
              << "Valid for [S:T]-inequalities: ['st', 'ST', 's:t', 'S:T']. "
              << "Skipping the current separator because these conditions were not met.";
        return;
      }
      auto ineq = elem["inequality"];
      // ### Δ inequalities
      if (ineq == "Δ" || ineq == "Triangle" || ineq == "triangle" || ineq == "T" || ineq == "t") {
        if (!elem.contains("maxcut") || !elem["maxcut"].is_integer()) {
          PLOGW << "Consider setting the 'maxcut' parameter when using a separator for Δ inequalities! "
                << "Defaulting to -1, i.e. no limit on the number of inequalities added per iteration.";
        }
        if (!elem.contains("variable_once") || !elem["variable_once"].is_boolean()) {
          PLOGW << "Consider setting the 'variable_once' flag when using a separator for Δ inequalities! "
                << "If set to 'true', for each variable there will be at most one Δ inequality containing that variable added per iteration. "
                << "Defaulting to 'false'.";
        }
        auto maxcut = elem["maxcut"].value_or(-1);
        auto var_once = elem["variable_once"].value_or(false);
        separator_configs.emplace_back(TriangleSeparatorConfig{tolerance, maxcut, var_once});
        PLOGD << name << ": Adding Δ-Separator with parameters: " <<
              "tolerance: " << tolerance << ", maxcut: " << maxcut << ", variable_once: "
              << (var_once ? "true" : "false") << ".";
      }
        // ### [S:T] inequalities
      else if (ineq == "st" || ineq == "ST" || ineq == "s:t" || ineq == "S:T") {
        if (!elem.contains("maxcut") || !elem["maxcut"].is_integer()) {
          PLOGW << "Consider setting the 'maxcut' parameter when using a separator for [S:T] inequalities! "
                << "Defaulting to -1, i.e. no limit on the number of inequalities added per iteration.";
        }
        StSeparatorHeuristic heuristic;
        if (!elem.contains("heuristic") || !elem["heuristic"].is_integer()) {
          PLOGW << "No heuristic was given for a [S:T]-separator in " << run_config_file << ". Assuming a default one.";
        }
        int h = elem["heuristic"].value_or(0);
        switch (h) {
          case 1: heuristic = StSeparatorHeuristic::GW1;
            break;
          case 2: heuristic = StSeparatorHeuristic::GW2;
            break;
          default: heuristic = StSeparatorHeuristic::GW1;
        }
        auto maxcut = elem["maxcut"].value_or(-1);
        separator_configs.emplace_back(StSeparatorConfig{tolerance, maxcut, heuristic});
        PLOGD << name << ": Adding st-Separator with parameters: " <<
              "tolerance: " << tolerance << ", maxcut: " << maxcut << ", heuristic: " << heuristic << ".";
      } else if (ineq == "odd" || ineq == "cycle" || ineq == "müller") {
        if (!elem.contains("maxcut") || !elem["maxcut"].is_integer()) {
          PLOGW << "Consider setting the 'maxcut' parameter when using a separator for [S:T] inequalities! "
                << "Defaulting to -1, i.e. no limit on the number of inequalities added per iteration.";
        }
        if (!elem.contains("type") || !elem["type"].is_string()) {
          PLOGW << "No inequality type was given for a cycle-separator in " << run_config_file
                << ". Assuming 2-chorded cycles.";
        }
        CircleInequality inequality_type;
        if (elem["type"] == "half-chorded") {
          inequality_type = CircleInequality::HALF_CHORDED;
        } else {
          inequality_type = CircleInequality::TWO_CHORDED;
        }
        auto maxcut = elem["maxcut"].value_or(-1);

        separator_configs.emplace_back(CircleSeparatorConfig{tolerance, maxcut, inequality_type});
        PLOGD << name << ": Adding cycle-Separator with parameters: " <<
              "tolerance: " << tolerance << ", maxcut: " << maxcut << ", inequality-type: " << inequality_type << ".";

      } else {
        PLOGW << run_config_file
              << ": Contains a table in the 'separators' entry with the 'inequality' attribute specified to "
              << ineq.as_string() << " but this inequality is not supported (yet).";
      }

    });
    if (seps.size() != separator_configs.size()) {
      PLOGW << "Only " << separator_configs.size() << " out of " << seps.size()
            << " entries of the 'separators' array could be parsed into separators. "
            << "Please make sure to use TOMLs array of table syntax (see template) for the separators. "
            << "Non-table entries are skipped during parsing.";
    }
  } catch (const toml::parse_error &err) {
    PLOGF << "Could not parse run_config.toml. Error was: " << err;
    exit(-1);
  }
}
