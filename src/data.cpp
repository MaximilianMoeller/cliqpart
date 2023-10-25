//
// Created by max on 31.07.23.
//

#include "data.h"
#include <plog/Log.h>
#include <string>
#include <toml++/toml.h>

DataConfig::DataConfig(const string &data_description_file) {
  toml::table tbl;

  try {
    tbl = toml::parse_file(data_description_file);

    // degree of the graph
    if (!tbl.contains("graph_degree") || !tbl["graph_degree"].is_integer()) {
      PLOGF << data_description_file << " must contain a key 'graph_degree' with an integer value "
                                        "specifying how many rows and columns of the data.csv should be "
                                        "used!";
      exit(-1);
    }
    graph_degree = tbl["graph_degree"].value_or(-1);
    PLOGD << "Graph degree set to " << graph_degree << ".";

    // optimization sense of the data
    if (!tbl.contains("maximizing") || !tbl["maximizing"].is_boolean()) {
      PLOGW << data_description_file << ": No optimization sense was given, assuming minimizing! "
            << "If you'd like to maximize instead, please use the key 'maximizing' and set it to 'true' in "
            << data_description_file << ".";
    }
    maximizing = tbl["maximizing"].value_or(false);
    PLOGD << "Optimization sense set to " << (maximizing ? "maximizing" : "minimizing") << ".";

    // row and column headers
    if (!tbl.contains("row_labels") || !tbl.contains("column_labels") || !tbl["row_labels"].is_integer()
        || !tbl["column_labels"].is_integer()) {
      PLOGW << "Please consider configuring the 'row_labels' and 'column_labels' "
               "values in the 'data_file' table in " << data_description_file << "! "
            << "Missing or non-integer values will be assumed 0, i.e. no row or column headers "
               "exist.";
    }
    row_labels = tbl["row_labels"].value_or(0);
    column_labels = tbl["column_labels"].value_or(0);
    PLOGD << "CSV data is assumed to have " << column_labels << " row(s) of column labels and " << row_labels
          << " column(s) of row labels.";

    // offset for edge costs
    if (!tbl.contains("value_offset")) {
      PLOGW << data_description_file << ": No value offset was given, assuming 0! "
            << "If you’d like to subtract a constant offset from all values in the "
               "CSV table, please use the key 'value_offset' in " << data_description_file << ".";
    }
    value_offset = tbl["value_offset"].value_or(0.0);
    PLOGD << "Values in CSV table will be offset by " << value_offset << ".";

    // scaling for edge costs
    if (!tbl.contains("value_scaling")) {
      PLOGW << data_description_file << ": No value scaling was given, assuming 1! "
            << "If you’d like to scale all objective values in the CSV table "
               "after offsetting them, please use the key 'value_scaling' in " << data_description_file << ".";
    }
    value_scaling = tbl["value_scaling"].value_or(1.0);
    PLOGD << "Values in CSV table will be scaled by " << value_scaling << ".";

    // scaling for edge costs
    if (!tbl.contains("integrality_tolerance")) {
      PLOGW << data_description_file << ": No integrality tolerance was given, assuming 1e-6! "
            << "If you'd like to terminate the algorithm with a different tolerance to integrality in the solution, "
               "consider setting the key 'integrality_tolerance' in " << data_description_file << ".";
    }
    integrality_tolerance = tbl["integrality_tolerance"].value_or(1e-6);
    PLOGD << "Solution integrality tolerance set to " << integrality_tolerance << ".";

  } catch (const toml::parse_error &err) {
    PLOGF << "Could not parse" << data_description_file << ". Error was: " << endl << err << endl;
    exit(-1);
  }
}
