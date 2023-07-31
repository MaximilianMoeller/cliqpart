#include "gurobi_c++.h"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include <plog/Log.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include "separator_factory.h"
#include "data.h"
#include "run_config.h"

using namespace std;

int main(int argc, char *argv[]) {

  CLI::App
	  app("Comparison Implementation of different heuristics for solving the LP-relaxation of CLIQUE PARTITIONING.");

  // Data dirs
  const static DataDirValidator kDataDirVal;
  vector<filesystem::path> data_dir_paths;
  app.add_option("DIRS",
				 data_dir_paths,
				 "Whitespace separated list of directories containing a data.csv and a data.toml file. (see README -> Usage).")->required(
	  true)->check(kDataDirVal)->expected(1, -1);

  // read in run_configs used for each run
  vector<string> run_config_paths;
  app.add_option("--config", run_config_paths, "Whitespace separated list of run_configs.")->expected(1, -1)->required(
	  true);

  CLI::Option *v = app.add_flag("-v", "Level of verbosity; increases with each use.");

  bool no_logs{false};
  app.add_flag("--no-logs", no_logs, "Enables logging to file, disabled by default.");

  bool gurobi_console_log{false};
  app.add_flag("-g", gurobi_console_log, "Enables gurobi console log, disabled by default.");

  bool lp_only{false};
  app.add_flag("--lp-only", gurobi_console_log, "Scip solving the ILP to optimality and only solve the LP relaxation");

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

  filesystem::path results_dir{"results/"};
  filesystem::create_directories(results_dir);

  // also display colored console logging
  static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;
  // log into a file called "log" before the first run and into "{run_dir}/log" afterward
  filesystem::path initial_log_file = results_dir / "log";
  if (!no_logs) {
	cout << "Setting logfile to " << initial_log_file.c_str() << endl;
  }
  static plog::RollingFileAppender<plog::TxtFormatter> file_appender(initial_log_file.c_str(), 16000000, 1000);

  if (no_logs) {
	plog::init(log_level, &console_appender);
  } else {
	// Initialize the logger
	plog::init(log_level, &file_appender).addAppender(&console_appender);
  }

  vector<RunConfig> run_configs{};
  for (auto run_config_path : run_config_paths) {
	RunConfig run_config{run_config_path};
	run_configs.emplace_back(run_config);
  }

  try {

	// environment can be kept and parametrized, only models need to be built for each run
	unique_ptr<GRBEnv> env = make_unique<GRBEnv>();
	env->set(GRB_IntParam_LogToConsole, gurobi_console_log);
	if (no_logs) {
	  env->set(GRB_StringParam_LogFile, "");
	}

	// everything else needs to be rebuilt for every base dir
	for (auto &data_dir_path : data_dir_paths) {

	  // TODO do the optimal solution here (and only if not already present)
	  auto basedir = results_dir / data_dir_path.parent_path().filename();
	  filesystem::create_directories(basedir);
	  for (const auto &kRunConfig : run_configs) {

		// ### BEGIN RUN SETUP ###
		auto const kNow = std::chrono::system_clock::now();
		auto const kInTimeT = std::chrono::system_clock::to_time_t(kNow);
		std::stringstream run_start_time;
		run_start_time << std::put_time(std::localtime(&kInTimeT), "%Y-%m-%d-%X");

		auto const kRunDir = basedir / string(kRunConfig.name + "-" + run_start_time.str());
		std::filesystem::create_directories(kRunDir);

		if (!no_logs) {
		  auto const kLogFile = kRunDir / "log";
		  PLOGD << "Setting logfile to " << kLogFile << endl;
		  file_appender.setFileName(kLogFile.c_str());
		}

		for (int run_counter = 1; run_counter <= kRunConfig.run_count; run_counter++) {

		  auto const kNumberedRunDir = kRunDir / to_string(run_counter);
		  std::filesystem::create_directories(kNumberedRunDir);
		  auto const kLogFile = kNumberedRunDir / "log";
		  auto const kGurobiLogFile = kNumberedRunDir / "gurobi.log";

		  if (!no_logs) {
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
		  DataConfig data_config{data_dir_path / "data.toml"};
		  ModelWrapper model_wrapper(*env, data_dir_path / "data.csv", data_config);

		  // generate separators based on run_config
		  vector<unique_ptr<IAbstractSeparator>>
			  separators = SeparatorFactory::BuildSeparator(kRunConfig, model_wrapper);

		  int constraints_added{0}, iteration{0};

		  // main LP loop, solving LP relaxation and adding violated constraints
		  do {
			iteration++;
			model_wrapper.optimize();
			model_wrapper.write(kNumberedRunDir / (to_string(iteration) + ".sol"));

			constraints_added = 0;
			for (auto &separator : separators) {
			  if (constraints_added > 0) break;
			  constraints_added = separator->AddCuts();
			}
			PLOGI << "Added " << constraints_added << " constraints in iteration " << iteration;

		  } while (constraints_added > 0);
		}

	  }
	}
  } catch (GRBException &e) {
	PLOGE << "Gurobi Error number: " << e.getErrorCode() << endl;
	PLOGE << e.getMessage() << endl;
  } catch (...) {
	PLOGF << "Unexpected error occurred." << endl;
  }

  return 0;
}
