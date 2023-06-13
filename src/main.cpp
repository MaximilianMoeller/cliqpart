/* Copyright 2023, Gurobi Optimization, LLC */

/* Solve a traveling salesman problem on a randomly generated set of
   points using lazy constraints.   The base MIP model only includes
   'degree-2' constraints, requiring each node to have exactly
   two incident edges.  Solutions to this model may contain subtours -
   tours that don't visit every node.  The lazy constraint callback
   adds new constraints to cut them off. */

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

bool colored_output;

int main(int argc, char *argv[]) {

  vector<string> runs;

  CLI::App app("Comparison Implementation of different branch-and-cut "
			   "algorithms for CLIQUE PARTITIONING.");

  app.add_option("RUNS", runs, "Whitespace separated list of run directories (see README -> Usage).")
	  ->required(true);

  CLI::Option *v = app.add_flag("-v", "Level of verbosity increases with every use.");

  CLI11_PARSE(app, argc, argv)

  plog::Severity log_level;

  switch (v->count()) {
	case 0:log_level = plog::Severity::none;
	  break;
	case 1:log_level = plog::Severity::fatal;
	  break;
	case 2:log_level = plog::Severity::error;
	  break;
	case 3:log_level = plog::Severity::warning;
	  break;
	case 4:log_level = plog::Severity::info;
	  break;
	case 5:log_level = plog::Severity::debug;
	  break;
	default: log_level = plog::Severity::verbose;
	  break;
  }

  for (string run : runs) {
	static plog::RollingFileAppender<plog::TxtFormatter> file_appender("log", 16000000, 1000); // Create the 1st appender.
	static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender; // Create the 2nd appender.

	plog::init(log_level,
			   &file_appender).addAppender(&console_appender); // Initialize the logger with the both appenders.

	try {

	  // only one GRBEnv is ever required
	  unique_ptr<GRBEnv> env{new GRBEnv};
	  GRBModel model = GRBModel(*env);

	  // Must set LazyConstraints parameter when using lazy constraints
	  model.set(GRB_IntParam_LazyConstraints, 1);

	  RunConfig config{.degree=5, .graph_data = "data/example.csv", .obj_offset = 0.5};
	  CompleteGraph graph(config, model);

	  // Generate separator based on runconfig
	  unique_ptr<AbstractSeparator> sep = SeparatorFactory::BuildSeparator(config, graph);
	  model.setCallback(sep.get());

	  // Optimize model
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

	} catch (GRBException e) {
	  PLOGE << "Error number: " << e.getErrorCode() << endl;
	  PLOGE << e.getMessage() << endl;
	} catch (...) {
	  PLOGF << "Error during optimization" << endl;
	}
  }

  return 0;
}
