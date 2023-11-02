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
  struct Node {
    bool uv;
    bool in_out;
    int i;
    int j;
  };

 protected:
  int degree_;
  int max_node_index_;

  // outer vector for all nodes
  // inner vector containing edges leaving this node in the format <weight, target_node>
  std::vector<std::vector<std::pair<double, int>>> weights_;

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

  [[nodiscard]] Node IndexToNode(int index) const {
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

 public:
  explicit AuxGraph(int degree) : degree_(degree) {
    max_node_index_ = MaxNodeIndex();
    weights_ = std::vector(max_node_index_ + 1, std::vector<std::pair<double, int>>());
  };

  virtual void AddArc(Node start, Node target, double weight);
  virtual std::pair<double, std::vector<Node>> ShortestPath(Node start, Node target) = 0;

  virtual ~AuxGraph() = default;
};

class HalfChordedAuxGraph : public AuxGraph {
 public:
  explicit HalfChordedAuxGraph(int degree) : AuxGraph(degree) {};
  // can use dijkstra because the graph only contains non-negative weights
  std::pair<double, std::vector<Node>> ShortestPath(Node start, Node target) override;
};

class TwoChordedAuxGraph : public AuxGraph {
 public:
  explicit TwoChordedAuxGraph(int degree) : AuxGraph(degree) {};
  // cannot use dijkstra because the graph does contains negative weights
  // however, no negative cycles are present which allows for some optimization in belman-fords-algorithm
  std::pair<double, std::vector<Node>> ShortestPath(Node start, Node target) override;

};

#endif //CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_
