/* Copyright 2023, Gurobi Optimization, LLC */

/* Solve a traveling salesman problem on a randomly generated set of
   points using lazy constraints.   The base MIP model only includes
   'degree-2' constraints, requiring each node to have exactly
   two incident edges.  Solutions to this model may contain subtours -
   tours that don't visit every node.  The lazy constraint callback
   adds new constraints to cut them off. */

#include "gurobi_c++.h"
#include <cstdlib>
#include <string>
#include <random>
#include <memory>
#include <algorithm>
#include <iterator>
#include <iostream>


using namespace std;

class triangleConstraints : public GRBCallback {
public:
    GRBVar **vars;
    int n;

    triangleConstraints(GRBVar **xvars, int xn) {
        vars = xvars;
        n = xn;
    }

protected:
    void callback() {
        try {
            if (where == GRB_CB_MIPSOL) {

                // iterate through whole graph to find a violated triangle
                for (int i = 0; i < n; ++i) {
                    for (int j = 0; j < i; ++j) {
                        for (int k = 0; k < j; ++k) {

                            // get the values of all the edge decisions
                            auto x_ij = vars[i][j];
                            auto x_ik = vars[i][k];
                            auto x_jk = vars[j][k];

                            // a triangle inequality is violated iff the sum of the decision-variables = 2;
                            // i.e. one edge is not in the clique partitioning while the other two are.
                            double sum = getSolution(x_ij) + getSolution(x_ik) + getSolution(x_jk);
                            // gurobi might return values slightly fractional
                            if (1.5 < sum && sum < 2.5) {
                                clog << "Found violated triangle: " << i << "-" << j << "-" << k << endl;
                                addLazy(x_ij + x_ik - x_jk <= 1);
                                addLazy(x_ij - x_ik + x_jk <= 1);
                                addLazy(-x_ij + x_ik + x_jk <= 1);
                                return;
                            }
                        }
                    }
                }
            }
        } catch (GRBException e) {
            cout << "Error number: " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        } catch (...) {
            cout << "Error during callback" << endl;
        }
    }

};

int
main(int argc,
     char *argv[]) {
    if (argc < 2) {
        cout << "Usage: cliqpart size" << endl;
        return 1;
    }
    // random numbers generator (copied from https://stackoverflow.com/a/19728404)
    const int min = -10, max = 10;
    std::random_device rd;     // Only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // Random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(min, max); // Guaranteed unbiased

    // A complete graph of size n has n*(n-1)/2 edges.
    int n = atoi(argv[1]);

    GRBVar **vars = NULL;
    int i;
    vars = new GRBVar *[n];
    for (i = 0; i < n; i++)
        vars[i] = new GRBVar[n];

    try {
        int j;

        // only one GRBEnv is ever required
        unique_ptr<GRBEnv> env{new GRBEnv};
        GRBModel model = GRBModel(*env);

        // Must set LazyConstraints parameter when using lazy constraints

        model.set(GRB_IntParam_LazyConstraints, 1);

        // Create binary decision variables
        // x_ij indicates whether the edge from node i to node j is in the clique partitioning or not
        for (i = 0; i < n; i++) {
            for (j = 0; j <= i; j++) {
                int weight = uni(rng);
                clog << "weight of x_" << i << j << ": " << weight << endl;
                vars[i][j] = model.addVar(0.0, 1.0, weight,
                                          GRB_BINARY, "x_" + to_string(i) + "_" + to_string(j));
                // completely symmetric
                vars[j][i] = vars[i][j];
            }
        }

        //// Degree-2 constraints

        //for (i = 0; i < n; i++) {
        //    GRBLinExpr expr = 0;
        //    for (j = 0; j < n; j++)
        //        expr += vars[i][j];
        //    model.addConstr(expr == 2, "deg2_" + to_string(i));
        //}

        // Forbid edge from node back to itself
        for (i = 0; i < n; i++)
            vars[i][i].set(GRB_DoubleAttr_UB, 0);

        // Set callback function
        //subtourelim cb = subtourelim(vars, n);
        triangleConstraints cb = triangleConstraints(vars, n);
        model.setCallback(&cb);

        // Optimize model

        model.optimize();

        // Extract solution

        if (model.get(GRB_IntAttr_SolCount) > 0) {
            double **sol = new double *[n];
            for (i = 0; i < n; i++)
                sol[i] = model.get(GRB_DoubleAttr_X, vars[i], n);

            cout << "Solution:" << endl;
            for (int i = 0; i < n; ++i) {
                cout << " \t" << i;
            }
            cout << endl << std::string((n + 1) * 8, '-') << endl;
            for (i = 0; i < n; i++) {
                cout << i << " |\t";
                std::copy(sol[i], sol[i] + n, std::ostream_iterator<int>(std::cout, "\t"));
                cout << endl;
            }

        }

    } catch (GRBException e) {
        cout << "Error number: " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) {
        cout << "Error during optimization" << endl;
    }

    for (i = 0; i < n; i++)
        delete[] vars[i];
    delete[] vars;
    return 0;
}
