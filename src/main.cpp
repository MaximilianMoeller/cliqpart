#include "gurobi_c++.h"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include "separators/triangle_separator.h"
#include "separators/abstract_separator.h"
#include "separator_factory.h"

#include <plog/Log.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>

using namespace std;

// validates (non-)existence of the needed files in the run_dir
struct ConfigDirValidator : public CLI::Validator {
  ConfigDirValidator() {
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

	  // RunDir should contain a "run_config.toml" …
	  helper = CLI::ExistingFile(str + "run_config.toml");
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
  // thus every run should be the path to a directory containing a "run_config.toml" and a "data.csv" file.
  vector<string> runs;

  CLI::App app("Comparison Implementation of different branch-and-cut "
			   "algorithms for CLIQUE PARTITIONING.");

  const static ConfigDirValidator kConfigDirVal;
  app.add_option("RUNS", runs, "Whitespace separated list of run directories (see README -> Usage).")
	->required(true)
	->check(kConfigDirVal);

  CLI::Option *v = app.add_flag("-v", "Level of verbosity increases with every use.");

  bool log_to_file{false};
  app.add_flag("-l", log_to_file, "Enables logging to file, disabled by default.");

  CLI11_PARSE(app, argc, argv)

  plog::Severity log_level;

  switch (v->count()) {
	case 0:log_level = plog::Severity::fatal;
	  break;
	case 1:log_level = plog::Severity::error;
	  break;
	case 2:log_level = plog::Severity::warning;
	  break;
	case 3:log_level = plog::Severity::info;
	  break;
	case 4:log_level = plog::Severity::debug;
	  break;
	default:log_level = plog::Severity::verbose;
	  break;
  }

  // also display colored console logging
  static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;
  // log into a file called "log" before the first run and into "{run_dir}/log" afterwards
  static plog::RollingFileAppender<plog::TxtFormatter> file_appender("log", 16000000, 1000);

  if (log_to_file) {
	// Initialize the logger
	plog::init(log_level, &file_appender).addAppender(&console_appender);
  } else {
	plog::init(log_level, &console_appender);
  }

  try {

	// environment can be kept and parametrized, only models need to be build for each run
	unique_ptr<GRBEnv> env{new GRBEnv};

	// everything else needs to be rebuilt for every run
	for (const string &kConfigDir : runs) {


	  // TODO run_config should allow rerunning the same config multiple times (for robustness in measurement)
	  // TODO therefore another loop over the this re-run-count is required
	  // TODO this should create directories named with timestamps and adjust the logfile again
	  RunConfig config = RunConfig::FromFile(kConfigDir + "run_config.toml");

	  for (int run_counter = 1; run_counter <= config.run_count; run_counter++) {


		// ### BEGIN RUN SETUP ###

		auto const now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		std::stringstream run_start_time;
		run_start_time << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%X");

		auto const kRunDir = kConfigDir + "run_" + run_start_time.str() + "/";
		std::filesystem::create_directories(kRunDir);

		if (log_to_file) {
		  auto const kLogFile = kRunDir + "log";
		  PLOGD << "Set logfile to " << kLogFile;
		  file_appender.setFileName(kLogFile.c_str());
		}

		// ### END RUN SETUP ###


		// load data and create model variables
		ModelWrapper model_wrapper(*env, config, kConfigDir + "data.csv");

		// generate separators based on run_config
		vector<unique_ptr<AbstractSeparator>> separators = SeparatorFactory::BuildSeparator(config, model_wrapper);

		int constraints_added, iteration{0};
		do {
		  iteration++;
		  model_wrapper.optimize();
		  model_wrapper.write(kRunDir + "solution_iteration_" + to_string(iteration) + ".sol");

		  constraints_added = 0;
		  for (unique_ptr<AbstractSeparator> &separator : separators) {
			if (constraints_added > 0) break;
			constraints_added = separator->add_Cuts();
		  }
		  PLOGI << "Added " << constraints_added << " constraints in iteration " << iteration;

		} while (constraints_added > 0);


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
	}
  } catch (GRBException
		   e) {
	PLOGE << "Error number: " << e.getErrorCode() << endl;
	PLOGE << e.getMessage() << endl;
  } catch (...) {
	PLOGF << "Unexpected error occurred." << endl;
  }

  return 0;
}
