//
// Created by max on 31.07.23.
//

#ifndef CLIQPART_SRC_DATA_H_
#define CLIQPART_SRC_DATA_H_

#include <variant>
#include <plog/Log.h>
#include <toml++/toml.h>
#include <string>
#include "CLI/Validators.hpp"
using namespace std;

// validates (non-)existence of the needed files in the base dir
struct DataDirValidator : public CLI::Validator {
  DataDirValidator() {
	name_ = "DataDir";
	func_ = [](const std::string &data_dir) {
	  string error_msg;

	  // the string should point to an existing directory.
	  error_msg = CLI::ExistingDirectory(data_dir);
	  if (!error_msg.empty()) { return "\n" + error_msg; }

	  string result;
	  // must contain a "data.csv" …
	  error_msg = CLI::ExistingFile(data_dir + "data.csv");
	  if (!error_msg.empty()) result += "\n" + error_msg;
	  // … and a "data.toml"
	  error_msg = CLI::ExistingFile(data_dir + "data.toml");
	  if (!error_msg.empty()) result += "\n" + error_msg;

	  return result;
	};
  }
};

struct DataConfig {
  // How many nodes of the CSV should be considered, i.e. enables only looking at a subgraph of the whole data.csv.
  int graph_degree{0};

  // Some data is meant for maximizing instead of minimizing.
  bool maximizing{false};

  // Some data might be labelled and those labels can be used in the naming ot the edge variables to facilitate debugging.
  int row_labels{0};
  int column_labels{0};

  // Offsetting each graph weight by this constant allows for the graph weights to be, e.g. all positive.
  double value_offset{0.0};
  // Fixing numerical inaccuracies can sometimes be accomplished by scaling all the weights by a constant factor.
  double value_scaling{1.0};

  explicit DataConfig(const string &data_description_file);

};
#endif //CLIQPART_SRC_DATA_H_
