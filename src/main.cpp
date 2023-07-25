#include "gurobi_c++.h"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
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

// validates (non-)existence of the needed files in the base dir
struct BaseDirValidator : public CLI::Validator {
  BaseDirValidator() {
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
	  }

	  // must contain a "data.csv".
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
  vector<string> baseDirs;

  CLI::App app("Comparison Implementation of different branch-and-cut "
			   "algorithms for CLIQUE PARTITIONING.");

  const static BaseDirValidator kBaseDirVal;
  app.add_option("DIRS", baseDirs, "Whitespace separated list of directories containing data and run configs (see README -> Usage).")
	->required(true)
	->check(kBaseDirVal);

  CLI::Option *v = app.add_flag("-v", "Level of verbosity; increases with each use.");

  bool log_to_file{false};
  app.add_flag("-l", log_to_file, "Enables logging to file, disabled by default.");

  bool gurobi_console_log{false};
  app.add_flag("-g", gurobi_console_log, "Enables gurobi console log, disabled by default.");

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

	// environment can be kept and parametrized, only models need to be built for each run
	unique_ptr<GRBEnv> env = make_unique<GRBEnv>();
	env->set(GRB_IntParam_LogToConsole, gurobi_console_log);
	if (!log_to_file) {
	  env->set(GRB_StringParam_LogFile, "");
	}

	// everything else needs to be rebuilt for every base dir
	for (const string &kBaseDir : baseDirs) {

	  // ### BEGIN RUN SETUP ###
	  auto const now = std::chrono::system_clock::now();
	  auto in_time_t = std::chrono::system_clock::to_time_t(now);
	  std::stringstream run_start_time;
	  run_start_time << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%X");

	  // {kBaseDir} (name provided by the user)
	  // |- data.csv
	  // |- run_config.toml
	  // |- {kRunDir} (name based on starting timestamp)
	  auto const kRunDir = kBaseDir + "_run_" + run_start_time.str() + "/";
	  std::filesystem::create_directories(kRunDir);

	  if (log_to_file) {
		// {kBaseDir}
		// |- data.csv
		// |- run_config.toml
		// |- {kRunDir}
		// |  |- log (for everything concerning the actual base dir (e.g. config parsing)
		auto const kLogFile = kRunDir + "log";
		PLOGD << "Setting logfile to " << kLogFile << endl;
		file_appender.setFileName(kLogFile.c_str());
	  }

	  /* find all ".toml" files in the current base directory, e.g.
	  //
	  // {kBaseDir}
	  // |- run_config.toml
	  // |- myIncrediblyComplexConfig.toml
	  // |- […]
	  // |- {kRunDir}
	  // |  |- log
	  */
	  vector<string> config_paths{};
	  for (auto &p : std::filesystem::recursive_directory_iterator(kBaseDir)) {
		if (p.path().extension() == ".toml") {
		  PLOGI << "Found config file " << p.path().stem().string() << " in " << kBaseDir << ".";
		  config_paths.emplace_back(p.path().string());
		}
	  }

	  vector<RunConfig> run_configs{};
	  for (const auto &config_path : config_paths) {
		PLOGI << "Parsing " << config_path << " in " << kBaseDir << ".";
		run_configs.emplace_back(RunConfig::FromFile(config_path));
	  }

	  /* Run each found ".toml" config file:
	  //
	  // {kBaseDir}
	  // |- run_config.toml
	  // |- myIncrediblyComplexConfig.toml
	  // |- […]
	  // |- {kRunDir}
	  // |  |- log
	  // |  |- {kConfigDir}
	  */
	  for (auto config : run_configs) {
		PLOGI << "Executing config '" << config.name << "'.";

		auto const kConfigDir = kRunDir + config.name + "/";
		std::filesystem::create_directories(kConfigDir);

		for (int run_counter = 1; run_counter <= config.run_count; run_counter++) {

		  /* Run each found ".toml" config file:
		  //
		  // {kBaseDir}
		  // |- run_config.toml
		  // |- myIncrediblyComplexConfig.toml
		  // |- […]
		  // |- {kRunDir}
		  // |  |- log
		  // |  |- {kConfigDir}
		  // |  |- 1
		  // |  |  | gurobi.log
		  // |  |  | log
		  // |  |  | solution_iteration_1.sol
		  // |  |- 2
		  // |  |  | gurobi.log
		  // |  |  | log
		  // |  |  | solution_iteration_1.sol
		  */
		  auto const kNumberedRunDir = kConfigDir + to_string(run_counter) + "/";
		  std::filesystem::create_directories(kNumberedRunDir);
		  auto const kLogFile = kNumberedRunDir + "log";
		  auto const kGurobiLogFile = kNumberedRunDir + "gurobi.log";

		  if (log_to_file) {
			// own logs
			PLOGD << "Setting logfile to " << kLogFile;
			file_appender.setFileName(kLogFile.c_str());

			// gurobi logs
			// need to create file manually
			std::ofstream outfile(kGurobiLogFile);
			env->set(GRB_StringParam_LogFile, kGurobiLogFile);
		  }

		  // ### END RUN SETUP ###

		  // load data and create model variables
		  ModelWrapper model_wrapper(*env, config, kBaseDir + "data.csv");

		  // generate separators based on run_config
		  vector<unique_ptr<AbstractSeparator>> separators = SeparatorFactory::BuildSeparator(config, model_wrapper);

		  int constraints_added, iteration{0};

		  // main LP loop, solving LP relaxation and adding violated constraints
		  do {
			iteration++;
			model_wrapper.optimize();
			model_wrapper.write(kNumberedRunDir + "solution_iteration_" + to_string(iteration) + ".sol");

			constraints_added = 0;
			for (unique_ptr<AbstractSeparator> &separator : separators) {
			  if (constraints_added > 0) break;
			  constraints_added = separator->add_Cuts();
			}
			PLOGI << "Added " << constraints_added << " constraints in iteration " << iteration;

		  } while (constraints_added > 0);
		}
	  }

	}
  } catch (GRBException &
  e) {
	PLOGE << "Gurobi Error number: " << e.getErrorCode() << endl;
	PLOGE << e.getMessage() << endl;
  } catch (...) {
	PLOGF << "Unexpected error occurred." << endl;
  }

  return 0;
}
