//
// Created by max on 04.10.23.
//

#ifndef CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_
#define CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_

#include <vector>
#include <memory>
#include <optional>
#include <limits>
#include "plog/Log.h"

class AuxGraph {
 public:
  int degree_;

  struct Node {
    bool uv;
    bool in_out;
    int i;
    int j;
  };

  explicit AuxGraph(int degree) : degree_(degree) {};

  [[nodiscard]] int NodeToIndex(Node node) const {
    if (node.i == node.j) {
      PLOGW << "Tried to convert auxiliary graph node to index, but node.i = node.j = " << node.i << "!";
      return -1;
    }
    int cell_ij = node.i * (degree_ - 1) + node.j - (node.i < node.j);
    return cell_ij * 4 + 2 * node.uv + node.in_out;
  }

  int MaxNodeIndex() {
    Node max_node{true, true, degree_, degree_ - 1};
    return NodeToIndex(max_node);
  }

  Node IndexToNode(int index) {
    if (index < 0) {
      PLOGW << "Tried to convert index to auxiliary graph node, but index was " << index << ".";
      return Node{};
    }
    bool in_out = index % 2;
    bool uv = index % 4 > 1;

    int cell_ij = index / 4;
    int i = cell_ij / (degree_ - 1);
    int rest = cell_ij % (degree_ - 1);
    int j = rest >= i ? rest + 1 : rest;

    return Node{uv, in_out, i, j};
  }

  virtual void AddArc(Node start, Node target, double weight) = 0;
  virtual ~AuxGraph() = default;
};

class TwoChordedAuxGraph : public AuxGraph {

 public:

  explicit TwoChordedAuxGraph(int degree) : AuxGraph(degree) {};

  void AddArc(Node start, Node target, double weight);
};

class HalfChordedAuxGraph : public AuxGraph {
 private:
  // outer vector for all nodes
  // inner vector containing edges leaving this node in the format <weight, target_node>
  std::vector<std::vector<std::pair<double, int>>> weights_;

 public:
  explicit HalfChordedAuxGraph(int degree)
      : AuxGraph(degree),
        weights_(MaxNodeIndex() + 1, std::vector<std::pair<double, int>>()) {};
  void AddArc(Node start, Node target, double weight) override;

  std::pair<double, std::vector<Node>> Dijkstra(Node start, Node target);
};

#endif //CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_
