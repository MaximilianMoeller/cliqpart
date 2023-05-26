//
// Created by max on 26.05.23.
//


#include <gurobi_c++.h>
#include <iostream>
#include <span>

#include "../types.h"

extern Verbosity verbosity;

using namespace std;

class Cubic_Triangle_Separator : public GRBCallback {
private:
    GRBVar **vars;
    int degree;
public:
    Cubic_Triangle_Separator(GRBVar **xvars, int xn) : vars(xvars), degree(xn) {};
protected:
    void callback() override {
        try {
            if (where == GRB_CB_MIPSOL) {

                // iterate through whole graph to find a violated triangle
                for (int i = 0; i < degree; ++i) {
                    for (int j = 0; j < i; ++j) {
                        // if kante = 1, gleichung eh nicht verletzt => continue
                        auto x_ij = vars[i][j];
                        auto v_ij = getSolution(x_ij);
                        if (v_ij > 0.9) continue;

                        // ggf. für ein paar (i,j) nur einen verletzten knoten suchen
                        for (int k = 0; k < j; ++k) {
                            // get the values of all the edge decisions
                            auto x_ik = vars[i][k];
                            auto v_ik = getSolution(x_ik);
                            auto x_jk = vars[j][k];
                            auto v_jk = getSolution(x_jk);

                            // a triangle inequality is violated iff the sum of the decision-variables = 2;
                            // i.e. one edge is not in the clique partitioning while the other two are.
                            double sum = v_ij + v_ik + v_jk;
                            // gurobi might return values slightly fractional
                            if (1.5 < sum && sum < 2.5) {
                                if (verbosity == Verbosity::Debug)
                                    clog << "Δ(" << i << "," << j << "," << k << ")" << endl;
                                if (verbosity == Verbosity::Trace)
                                    clog << "Δ(" << endl <<
                                         "\t" << i << "-" << j << ":" << v_ij << endl <<
                                         "\t" << i << "-" << k << ":" << v_ik << endl <<
                                         "\t" << j << "-" << k << ":" << v_jk << endl <<
                                         ")" << endl;
                                if (v_jk < 0.9)
                                    addLazy(x_ij + x_ik - x_jk <= 1);
                                else if (v_ik < 0.9)
                                    addLazy(x_ij - x_ik + x_jk <= 1);
                                else if (v_ij < 0.9)
                                    addLazy(-x_ij + x_ik + x_jk <= 1);
                            }
                        }
                    }
                }
            }
        } catch (GRBException &e) {
            cout << "Error number: " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        } catch (...) {
            cout << "Error during callback" << endl;
        }
    }
};