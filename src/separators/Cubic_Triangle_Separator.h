//
// Created by max on 30.05.23.
//

#ifndef CLIQPART_CUBIC_TRIANGLE_SEPARATOR_H
#define CLIQPART_CUBIC_TRIANGLE_SEPARATOR_H

#include <gurobi_c++.h>

using namespace std;

class Cubic_Triangle_Separator : public GRBCallback {
private:
    GRBVar **vars;
    int degree;
public:
    Cubic_Triangle_Separator(GRBVar **xvars, int xn) : vars(xvars), degree(xn) {}

protected:
    void callback() override;
};

#endif //CLIQPART_CUBIC_TRIANGLE_SEPARATOR_H
