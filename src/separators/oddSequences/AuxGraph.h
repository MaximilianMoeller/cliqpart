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
 protected:
  int max_node_index_;

 public:
  int degree_;

  struct Node {
    bool uv;
    bool in_out;
    int i;
    int j;
  };

  // hacky, but easiest thing to do
  class OutOfTimeException : public std::exception {
   public:
    const char *what() {
      return "Run out of time.";
    };
  };

  explicit AuxGraph(int degree) : degree_(degree) { max_node_index_ = MaxNodeIndex(); };

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

  virtual void AddArc(Node start, Node target, double weight) = 0;
  virtual std::pair<double, std::vector<Node>> shortestPath(Node start, Node target) = 0;

  virtual ~AuxGraph() = default;
};

class HalfChordedAuxGraph : public AuxGraph {
 private:
  // outer vector for all nodes
  // inner vector containing edges leaving this node in the format <weight, target_node>
  std::vector<std::vector<std::pair<double, int>>> weights_;

 public:
  explicit HalfChordedAuxGraph(int degree)
      : AuxGraph(degree) {
    weights_ = std::vector(max_node_index_ + 1, std::vector<std::pair<double, int>>());
  };

  void AddArc(Node start, Node target, double weight) override;

  // can use dijkstra because the graph only contains non-negative weights
  std::pair<double, std::vector<Node>> shortestPath(Node start, Node target) override;
};

class TwoChordedAuxGraph : public AuxGraph {
 private:
  const int time_limit_{-1};
  bool floyd_warshall_computed_{false};
  std::vector<std::vector<double>> weights_;
  std::vector<std::vector<double>> dist_;
  std::vector<std::vector<int>> present_arcs_;
  std::vector<std::vector<int>> pred_;

 public:

  explicit TwoChordedAuxGraph(int degree, int time_limit) :
      AuxGraph(degree), time_limit_(time_limit) {
    weights_ = std::vector(max_node_index_ + 1,
                           std::vector<double>(max_node_index_ + 1, std::numeric_limits<double>::infinity()));
    dist_ = std::vector(max_node_index_ + 1,
                        std::vector<double>(max_node_index_ + 1, std::numeric_limits<double>::infinity()));
    pred_ = std::vector(max_node_index_ + 1, std::vector<int>(max_node_index_ + 1, -1));
    present_arcs_ = std::vector(max_node_index_ + 1, std::vector<int>());
  }

  void AddArc(Node start, Node target, double weight)
  override;

  void FloydWarshall();
  // floyd warshall is faster than doing belman-ford n^2 times
  // cannot use dijkstra, because graph contains negative edges
  std::pair<double, std::vector<Node>> shortestPath(Node start, Node target)
  override;
};

#endif //CLIQPART_SRC_SEPARATORS_ODDSEQUENCES_AUXGRAPH_H_
