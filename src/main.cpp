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
#include "verbosity.h"
#include "separators/abstract_separator.h"
#include "separator_factory.h"

using namespace std;

int main(int argc, char *argv[]) {

  Verbosity verbosity;

  CLI::App app("Comparison Implementation of different branch-and-cut "
			   "algorithms for CLIQUE PARTITIONING.");
  CLI::Option *v = app.add_flag("-v", "Level of verbosity_ increases with every use.");

  CLI11_PARSE(app, argc, argv)

  switch (v->count()) {
	case 0:verbosity = Verbosity::ERROR;
	  break;
	case 1:verbosity = Verbosity::WARN;
	  break;
	case 2:verbosity = Verbosity::INFO;
	  break;
	case 3:verbosity = Verbosity::DEBUG;
	  break;
	default:
	  // for 4 or higher
	  verbosity = Verbosity::TRACE;
	  break;
  }
  cout << "Set verbosity_ level to {" << verbosity << "}." << endl;

  // random number sampling ersetzen durch „sinnvolle“ daten, z.b. erst cluster
  // erzeugen und dann von 2 gebiasden verteilungen samplen

  // seeden für Vergleichbarkeit

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
	cout << "Error number: " << e.getErrorCode() << endl;
	cout << e.getMessage() << endl;
  } catch (...) {
	cout << "Error during optimization" << endl;
  }

  return 0;
}
