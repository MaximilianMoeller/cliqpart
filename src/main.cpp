/* Copyright 2023, Gurobi Optimization, LLC */

/* Solve a traveling salesman problem on a randomly generated set of
   points using lazy constraints.   The base MIP model only includes
   'degree-2' constraints, requiring each node to have exactly
   two incident edges.  Solutions to this model may contain subtours -
   tours that don't visit every node.  The lazy constraint callback
   adds new constraints to cut them off. */

#include "gurobi_c++.h"
#include <string>
#include <random>
#include <memory>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <CLI/CLI.hpp>
#include <CLI/App.hpp>

#include "separators/Cubic_Triangle_Separator.h"
#include "types.h"

using namespace std;

Verbosity verbosity;

int
main(int argc, char *argv[]) {

    // Degree of the complete graph, i.e. number of vertices.
    int degree;

    CLI::App app("Comparison Implementation of different branch-and-cut algorithms for CLIQUE PARTITIONING.");
    CLI::Option *v = app.add_flag("-v", "Level of verbosity increases with every use.");
    app.add_option("DEGREE", degree, "The degree of the complete graph")
            ->required(false)
            ->default_val(15);
    CLI11_PARSE(app, argc, argv);

    switch (v->count()) {
        case 0:
            verbosity = Verbosity::Error;
            break;
        case 1:
            verbosity = Verbosity::Warn;
            break;
        case 2:
            verbosity = Verbosity::Info;
            break;
        case 3:
            verbosity = Verbosity::Debug;
            break;
        default:
            // for 4 or higher
            verbosity = Verbosity::Trace;
            break;
    }
    cout << "Set verbosity level to {" << verbosity << "}." << endl;

    // random number sampling ersetzen durch „sinnvolle“ daten, z.b. erst cluster erzeugen und dann von 2 gebiasden verteilungen samplen


    // random numbers generator (copied from https://stackoverflow.com/a/19728404)
    // seeden für Vergleichbarkeit
    const int min = -5, max = 15;
    std::random_device rd;     // Only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // Random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(min, max); // Guaranteed unbiased

    GRBVar **vars = nullptr;
    int i;
    // A complete graph of degree n has n*(n-1)/2 edges.
    vars = new GRBVar *[degree];
    for (i = 0; i < degree; i++)
        vars[i] = new GRBVar[degree];

    try {
        int j;

        // only one GRBEnv is ever required
        unique_ptr<GRBEnv> env{new GRBEnv};
        GRBModel model = GRBModel(*env);

        // Must set LazyConstraints parameter when using lazy constraints

        model.set(GRB_IntParam_LazyConstraints, 1);

        // Create binary decision variables
        // x_ij indicates whether the edge from node i to node j is in the clique partitioning or not
        for (i = 0; i < degree; i++) {
            for (j = 0; j <= i; j++) {
                //int weight = uni(rng) ? 1 : -1;
                //if (i == j) continue;
                int weight = uni(rng);
                if (verbosity >= Verbosity::Debug)
                    clog << "weight of x_" << i << j << ": " << weight << endl;
                vars[i][j] = model.addVar(0.0, 1.0, weight,
                                          GRB_BINARY, "x_" + to_string(i) + "_" + to_string(j));
                // completely symmetric
                vars[j][i] = vars[i][j];
            }
        }

        // Forbid edge from node back to itself
        for (i = 0; i < degree; i++)
            vars[i][i].set(GRB_DoubleAttr_UB, 0);

        // Set callback function
        Cubic_Triangle_Separator cb = Cubic_Triangle_Separator(vars, degree);
        model.setCallback(&cb);

        // Optimize model

        model.optimize();

        // Extract solution

        if (model.get(GRB_IntAttr_SolCount) > 0) {
            double **sol = new double *[degree];
            for (i = 0; i < degree; i++)
                sol[i] = model.get(GRB_DoubleAttr_X, vars[i], degree);

            cout << "Solution:" << endl << "\t";
            for (int i = 0; i < degree; ++i) {
                cout << i << "\t";
            }
            cout << endl << std::string((degree + 1) * 8, '-') << endl;
            for (i = 0; i < degree; i++) {
                cout << i << "|\t";
                std::copy(sol[i], sol[i] + degree, std::ostream_iterator<int>(std::cout, "       "));
                cout << endl;
            }

        }

    } catch (GRBException e) {
        cout << "Error number: " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) {
        cout << "Error during optimization" << endl;
    }

    for (i = 0; i < degree; i++)
        delete[] vars[i];
    delete[] vars;
    return 0;
}
