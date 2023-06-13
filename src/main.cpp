#include "gurobi_c++.h"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include "separators/cubic_triangle_separator.h"
#include "separators/abstract_separator.h"
#include "separator_factory.h"

#include <plog/Log.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>

using namespace std;

// validates (non-)existence of the needed files in the run_dir
struct RunDirValidator : public CLI::Validator {
  RunDirValidator() {
	name_ = "RUNDIR";
	func_ = [](const std::string &str) {
	  string result;
	  string helper;

	  // the string should point to an existing directory.
	  helper = CLI::ExistingDirectory(str);
	  if (!helper.empty()) return "\n" + helper;
	  // make sure the run directory ends in '/' otherwise the following checks would contain '//'.
	  if (str.back() != '/') {
		return string("\nRun directory must be specified with trailing '/': " + str);
	  };

	  // RunDir should contain a "run_config.yaml" …
	  helper = CLI::ExistingFile(str + "run_config.yaml");
	  if (!helper.empty()) result += "\n" + helper;
	  // … and a "data.csv".
	  helper = CLI::ExistingFile(str + "data.csv");
	  if (!helper.empty()) result += "\n" + helper;

	  return result;
	};
  }
};

int main(int argc, char *argv[]) {

  // can run different settings after each other
  // every such run is dependent on a few configuration parameters (e.g. selection of heuristics used) and data,
  // thus every run should be the path to a directory containing a "run_config.yaml" and a "data.csv" file.
  vector<string> runs;

  CLI::App app("Comparison Implementation of different branch-and-cut "
			   "algorithms for CLIQUE PARTITIONING.");

  const static RunDirValidator kRunDirVal;
  app.add_option("RUNS", runs, "Whitespace separated list of run directories (see README -> Usage).")
	  ->required(true)
	  ->check(kRunDirVal);

  CLI::Option *v = app.add_flag("-v", "Level of verbosity increases with every use.");

  CLI11_PARSE(app, argc, argv)

  plog::Severity log_level;

  switch (v->count()) {
	case 6:log_level = plog::Severity::verbose;
	  break;
	case 5:log_level = plog::Severity::debug;
	  break;
	case 4:log_level = plog::Severity::info;
	  break;
	case 3:log_level = plog::Severity::warning;
	  break;
	case 2:log_level = plog::Severity::error;
	  break;
	case 1:log_level = plog::Severity::fatal;
	  break;
	default: log_level = plog::Severity::none;
	  break;
  }

  // log into a file called "log" before the first run and into "{run_dir}/log" afterwards
  static plog::RollingFileAppender<plog::TxtFormatter> file_appender("log", 16000000, 1000);
  // also display colored console logging
  static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;

  // Initialize the logger
  plog::init(log_level,
			 &file_appender).addAppender(&console_appender);

  try {
	// everything else needs to be rebuilt for every run
	for (const string &kRunDir : runs) {
	  // set log file for this run
	  const string kLogFile = kRunDir + "log";
	  file_appender.setFileName(kLogFile.c_str());

	  // TODO load run_config.yaml here
	  // TODO run_config should allow rerunning the same config multiple times (for robustness in measurement)
	  // TODO therefore another loop over the this re-run-count is required
	  // TODO this should create directories named with timestamps and adjust the logfile again
	  RunConfig config{.degree=5, .graph_data = "data/example.csv", .obj_offset = 0.5};

	  // always create a new environment instead of reusing the old one to ensure fairness between runs
	  unique_ptr<GRBEnv> env{new GRBEnv};
	  GRBModel model = GRBModel(*env);
	  // must set LazyConstraints parameter when using lazy constraints
	  model.set(GRB_IntParam_LazyConstraints, 1);

	  // load data and create model variables
	  CompleteGraph graph(config, model);

	  // generate separator based on run_config
	  unique_ptr<AbstractSeparator> sep = SeparatorFactory::BuildSeparator(config, graph);
	  model.setCallback(sep.get());

	  // optimize model
	  model.optimize();

	  // Extract solution

//	if (model.get(GRB_IntAttr_SolCount) > 0) {
//	  double **sol = new double *[degree];
//	  for (int i = 0; i < degree; i++)
//		sol[i] = model.get(GRB_DoubleAttr_X, vars[i], degree);
//
//	  cout << "Solution:" << endl << "\t";
//	  for (int i = 0; i < degree; ++i) {
//		cout << i << "\t";
//	  }
//	  cout << endl << std::string((degree + 1) * 8, '-') << endl;
//	  for (int i = 0; i < degree; i++) {
//		cout << i << "|\t";
//		std::copy(sol[i], sol[i] + degree,
//				  std::ostream_iterator<int>(std::cout, "       "));
//		cout << endl;
//	  }
//	}

	}
  } catch (GRBException
		   e) {
	PLOGE << "Error number: " << e.getErrorCode() << endl;
	PLOGE << e.getMessage() << endl;
  } catch (...) {
	PLOGF << "Error during optimization" << endl;
  }

  return 0;
}
