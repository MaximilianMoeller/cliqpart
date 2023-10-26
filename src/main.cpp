#include "gurobi_c++.h"
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

#include <plog/Log.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Formatters/CsvFormatter.h>
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include "separator_factory.h"
#include "data.h"
#include "run_config.h"
#include "ilp_callback.h"

using namespace std;

enum {
  CSVLog = 1
};

int main(int argc, char *argv[]) {

  // command line arguments
  CLI::App app
      ("Comparison implementation of different heuristic separation routines for solving the LP-relaxation of CLIQUE PARTITIONING.");

  // data dirs
  const static DataDirValidator kDataDirVal;
  vector<filesystem::path> data_dir_paths;
  app.add_option("DIRS",
                 data_dir_paths,
                 "Whitespace separated list of directories containing a data.csv and a data.toml file. (see README -> Usage).")
      ->required(true)->check(kDataDirVal)->expected(1, -1);

  // read in run_config paths
  vector<string> run_config_paths;
  app.add_option("-c,--config", run_config_paths, "Whitespace separated list of run_configs.")
      ->expected(1, -1)->required(true);

  CLI::Option *v = app.add_flag("-v,--verbose", "Verbose; use twice for even more details.");

  bool log_to_file{false};
  app.add_flag("-f,--log-to-file", log_to_file, "Enable logging to file, disabled by default.");

  bool gurobi_console_log{false};
  app.add_flag("-g,--gurobi-logs", gurobi_console_log, "Enables gurobi log in console, disabled by default.");

  bool lp_only{false};
  app.add_flag("-l,--lp-only", lp_only, "Scip solving the ILP to optimality and only solve the LP relaxation.");

  app.usage("Usage: cliqpart DIRS --config CONFIGS [OPTIONS]");

  CLI11_PARSE(app, argc, argv)

  plog::Severity log_level;
  switch (v->count()) {
    case 0:log_level = plog::Severity::info;
      break;
    case 1:log_level = plog::Severity::debug;
      break;
    default:log_level = plog::Severity::verbose;
      break;
  }

  // setup of results and logging
  filesystem::path results_dir{"results/"};
  filesystem::create_directories(results_dir);
  filesystem::path main_log_file_path = results_dir / "log";
  // clear previous log file
  std::ofstream main_logfile{main_log_file_path, std::ios::out | std::ios::trunc};
  main_logfile.close();

  // display colored console logging
  static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;
  static plog::RollingFileAppender<plog::TxtFormatter> log_file_appender(main_log_file_path.c_str(), 16000000, 1);
  if (log_to_file) {
    // initialize the console logger and the file logger
    plog::init(log_level, &log_file_appender).addAppender(&console_appender);
  } else {
    // initialize only the console logger
    plog::init(log_level, &console_appender);
  }

  // setup of measurements logging
  filesystem::path measurements_file_path = results_dir / "measurements.csv";
  std::ofstream measurements_file{measurements_file_path, std::ios::out | std::ios::trunc};
  measurements_file.close();
  static plog::RollingFileAppender<plog::CsvFormatter>
      measurements_file_appender(measurements_file_path.c_str(), 16000000, 1);
  plog::init<CSVLog>(plog::info, &measurements_file_appender);

  // loading configurations
  PLOGD << "Starting parsing of configuration file(s).";
  vector<RunConfig> run_configs{};
  for (auto &run_config_path : run_config_paths) {
    RunConfig run_config{run_config_path};
    run_configs.emplace_back(run_config);
  }
  PLOGD << "Finished parsing of configuration file(s).";

  try {

    // gurobi environment can be kept and parametrized, only models need to be built for each run
    unique_ptr<GRBEnv> env = make_unique<GRBEnv>(true);
    // disable gurobi output by default, especially the 'academic license […]' print
    env->set(GRB_IntParam_OutputFlag, true);
    env->set(GRB_IntParam_LogToConsole, gurobi_console_log);
    env->start();
    if (!log_to_file) {
      env->set(GRB_StringParam_LogFile, "");
    }

    // everything else needs to be rebuilt for every base dir
    for (auto &data_dir_path : data_dir_paths) {
      auto data_name = data_dir_path.parent_path().filename();
      PLOGI << "Starting computations for data set '" << data_name << "'.";

      // one base directory per data directory
      auto basedir = results_dir / data_name;
      filesystem::create_directories(basedir);
      if (log_to_file) {
        auto const kLogFile = basedir / "log";
        // clear previous log file
        std::ofstream logfile{kLogFile, std::ios::out | std::ios::trunc};
        logfile.close();

        PLOGD << "Setting logfile to " << kLogFile << endl;
        log_file_appender.setFileName(kLogFile.c_str());
      }

      // load data description file
      DataConfig data_config{data_dir_path / "data.toml"};

      // only solve to optimality if it has neither been disabled by the user
      // nor has been solved before
      if (!lp_only && (!filesystem::exists(data_dir_path / "optimal.sol")
          || !filesystem::exists(data_dir_path / "measurements.csv"))) {
        PLOGI << "No optimal solution was found for data set '" << data_dir_path
              << "'. Starting solving to optimality.";
        if (log_to_file) {
          auto const kLogFile = data_dir_path / "optimal.log";
          // clear previous log file
          std::ofstream logfile{kLogFile, std::ios::out | std::ios::trunc};
          logfile.close();

          PLOGD << "Setting logfile to " << kLogFile << endl;
          log_file_appender.setFileName(kLogFile.c_str());

          auto const kGurobiLogFile = data_dir_path / "gurobi.log";
          std::ofstream outfile(kGurobiLogFile);
          env->set(GRB_StringParam_LogFile, kGurobiLogFile);
        }

        PLOGI << "Creating ILP model.";
        CliquePartModel ilp_model{*env, data_dir_path / "data.csv", data_config, false};
        ilp_model.set(GRB_IntParam_LazyConstraints, 1);

        ILPCallback ilp_callback{ilp_model};
        ilp_model.setCallback(&ilp_callback);

        PLOGI << "Starting optimal solving";

        measurements_file_appender.setFileName((data_dir_path / "measurements.csv").c_str());
        PLOGI_(CSVLog) << "Start optimal solve";
        ilp_model.optimize();
        PLOGI_(CSVLog) << "End optimal solve";

        ilp_model.write(data_dir_path / "optimal.sol");
        PLOGI << "Finished optimal solving. Optimal objective value is: "
              + to_string(ilp_model.get(GRB_DoubleAttr_ObjVal));
      }

      for (const auto &kRunConfig : run_configs) {
        PLOGI << "Starting run config '" << kRunConfig.name << "' for data set '" << data_dir_path << "'.";

        // ### BEGIN RUN SETUP ###
        auto const kNow = std::chrono::system_clock::now();
        auto const kInTimeT = std::chrono::system_clock::to_time_t(kNow);
        std::stringstream run_start_time;
        run_start_time << std::put_time(std::localtime(&kInTimeT), "%Y-%m-%d-%X");

        auto const kRunDir = basedir / string(kRunConfig.name + "-" + run_start_time.str());
        std::filesystem::create_directories(kRunDir);

        if (log_to_file) {
          auto const kLogFile = kRunDir / "log";
          PLOGD << "Setting logfile to " << kLogFile << endl;
          log_file_appender.setFileName(kLogFile.c_str());
        }

        for (int run_counter = 1; run_counter <= kRunConfig.run_count; run_counter++) {
          PLOGI << "Starting run " << run_counter << "/" << kRunConfig.run_count << " for run config '"
                << kRunConfig.name << "' and data set '" << data_dir_path << "'.";

          auto const kNumberedRunDir = kRunDir / to_string(run_counter);
          std::filesystem::create_directories(kNumberedRunDir);

          if (log_to_file) {
            auto const kLogFile = kNumberedRunDir / "log";
            auto const kGurobiLogFile = kNumberedRunDir / "gurobi.log";
            // own logs
            PLOGD << "Setting logfile to " << kLogFile;
            log_file_appender.setFileName(kLogFile.c_str());

            // gurobi logs
            // need to create file manually
            std::ofstream outfile(kGurobiLogFile);
            env->set(GRB_StringParam_LogFile, kGurobiLogFile);
          }

          // ### END RUN SETUP ###

          // generate separators based on run_config
          vector<unique_ptr<IAbstractSeparator>>
              separators = SeparatorFactory::BuildSeparator(data_config.graph_degree, kRunConfig);

          // load data and create model variables
          CliquePartModel model{*env, data_dir_path / "data.csv", data_config};

          int iteration{0};
          vector<GRBTempConstr> violated_constraints;

          // main LP loop, solving LP relaxation and adding violated constraints
          measurements_file_appender.setFileName((kNumberedRunDir / "measurements.csv").c_str());
          do {
            // solving LP and writing solution to file
            iteration++;
            model.optimize();
            model.write(kNumberedRunDir / (to_string(iteration) + ".sol"));

            // enumerating violated constraints
            violated_constraints = {};
            for (auto &separator : separators) {
              violated_constraints = separator->SeparateSolution(model.GetSolution(), model.GetVars());

              // if the triangle separator found no violated constraints and the solution is integral
              // there is no need to search for other violated inequalities -> abort early
              if (separator->Abbreviation() == "Δ" && violated_constraints.empty() && model.IsIntegral()) {
                PLOGI << "Found integral solution in iteration " << iteration << ".";
                // very clear usage of goto, don't blame me
                goto integral_solution;
              }

              // if a separator found violated constraints, we want to add those to the model and optimize again
              // before calling other separators, because they might depend on the fact that a specific class
              // of inequalities is satisfied.
              // In fact, ALL other implemented separators use the assumption that the triangle inequalities are
              // satisfied.
              if (!violated_constraints.empty()) {
                PLOGI_(CSVLog) << "{\"iteration\":" << iteration
                               << ",\"obj_value\":" << model.get(GRB_DoubleAttr_ObjVal)
                               << ",\"violated_found\":" << violated_constraints.size()
                               << ",\"separator\":\"" << separator->Abbreviation() // NOLINT(*-raw-string-literal)
                               << "\"}";
                break;
              }
            }

            // adding violated constraints to model
            for (const auto &constraint : violated_constraints) {
              model.addConstr(constraint);
            }
            PLOGD << "Added " << violated_constraints.size() << " constraints in iteration " << iteration;

          } while (!violated_constraints.empty());
          integral_solution:;
          PLOGI_(CSVLog) << "{\"iteration\":" << iteration
                         << ",\"obj_value\":" << model.get(GRB_DoubleAttr_ObjVal)
                         << ",\"violated_found\":" << 0
                         << ",\"separator\":\"" << "none" // NOLINT(*-raw-string-literal)
                         << "\"}";

          model.write(kNumberedRunDir / "0_last.sol");
          PLOGI << "Finished run " << run_counter << "/" << kRunConfig.run_count << " for run config '"
                << kRunConfig.name << "' and data set '" << data_dir_path << "'.";
          PLOGI << "Took " << iteration << " iterations and obtained objective value is "
                << to_string(model.get(GRB_DoubleAttr_ObjVal));
        }
        PLOGI << "Finished run config '" << kRunConfig.name << "' for data set '" << data_dir_path << "'.";
      }
      PLOGI << "Finished all computations for data set '" << data_dir_path << "'.";
    }
  } catch (GRBException &e) {
    PLOGE << "Gurobi Error number: " << e.getErrorCode() << endl;
    PLOGE << e.getMessage() << endl;
  } catch (...) {
    PLOGF << "Unexpected error occurred." << endl;
  }

  return 0;
}
