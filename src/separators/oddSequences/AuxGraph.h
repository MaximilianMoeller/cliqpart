//
// Created by max on 04.10.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_
#define CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_

#include <vector>
#include <memory>
#include <optional>

class AuxGraph {

 public:
  struct Node {
    bool uv;
    bool in_out;
    int i;
    int j;
  };

  explicit AuxGraph(int degree) : degree_(degree) {
    Node max = {true, true, degree_, degree_ - 1};
    max_node_index_ = NodeToIndex(max);
    arcs_ = std::vector<std::vector<double>>(max_node_index_,
                                             std::vector<double>(max_node_index_, INFINITY));
  };

  void AddArc(Node start, Node target, double weight);


  // call FloydWarshall first, then dist_ and prev_ will be populated
  // from which the weights and paths can be derived
  void FloydWarshall();
  double getWeight(Node s, Node t);
  std::vector<Node> Path(Node s, Node t);

  std::pair<double, std::vector<Node>> BellmanFord(Node s, Node t);




 private:
  int degree_;
  int max_node_index_;
  std::vector<std::vector<double>> arcs_;
  std::vector<std::vector<double>> dist_;
  std::vector<std::vector<int>> prev_;

  [[nodiscard]] int NodeToIndex(Node s) const {
    return (s.i * degree_ + s.j) * 4 + s.in_out * 2 + s.uv;
  }
  [[nodiscard]] Node IndexToNode(int index) const {
    if (index > max_node_index_ || index < 0) exit(-1);
    bool uv = (index % 2 == 1);
    bool io = (index % 4 > 1);
    int i = (index / 4) / degree_;
    int j = (index / 4) % degree_;

    return Node{uv, io, i, j};
  }

};

#endif //CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_
