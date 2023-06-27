//
// Created by max on 09.06.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
#define CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_

#include "../complete_graph.h"
#include <gurobi_c++.h>

using namespace std;

class AbstractSeparator : public GRBCallback {
 private:
  CompleteGraph &graph_;
  double *current_solution_{nullptr};
  void FetchSolution();;

 protected:
  explicit AbstractSeparator(CompleteGraph &graph) : graph_(graph) {}
  void callback() final;;
  virtual GRBVar GetVar(int i, int j) final {
	return graph_.GetVar(i, j);
  }
  virtual int GetDegree() final {
	return graph_.GetDegree();
  }
  virtual int EdgeCount() final {
	return graph_.EdgeCount();
  }
  virtual void my_callback() = 0;
  double getSolution(int i, int j);

 public:
  ~AbstractSeparator() override {
	delete[] current_solution_;
	GRBCallback::~GRBCallback();
  }
};

#endif // CLIQPART_SRC_SEPARATORS_ABSTRACT_SEPARATOR_H_
