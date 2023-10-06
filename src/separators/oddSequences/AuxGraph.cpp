//
// Created by max on 04.10.23.
//

#include <cmath>
#include <algorithm>
#include <iostream>
#include "AuxGraph.h"
#include "plog/Log.h"
void AuxGraph::AddArc(Node s, Node t, double weight) {
  if ((s.i == s.j) || (t.i == t.j)) return;
  arcs_[NodeToIndex(s)][NodeToIndex(t)] = weight;

}
void AuxGraph::FloydWarshall() {
  PLOGV << "Entering FloydWarshall";

  // Initialize data structures
  dist_ = std::vector<std::vector<double>>(max_node_index_,
                                           std::vector<double>(max_node_index_, INFINITY));
  prev_ = std::vector<std::vector<int>>(max_node_index_,
                                        std::vector<int>(max_node_index_, -1));

  PLOGV << " 1";
  for (int i = 0; i < max_node_index_; ++i) {
    for (int j = 0; j < max_node_index_; ++j) {
      if (i == j) continue;
      if (arcs_[i][j] < INFINITY) {
        dist_[i][j] = arcs_[i][j];
        prev_[i][j] = i;

      }
    }
  }
  PLOGV << " 2";

  for (int i = 0; i < max_node_index_; ++i) {
    float i_progress = (float) i / (float) max_node_index_;
    for (int j = 0; j < max_node_index_; ++j) {
      float j_progress = (float) j / (float) max_node_index_;
      std::cout << "(" << i_progress * 100 << ", " << j_progress * 100 <<") %\r";
      std::cout.flush();
      if (i == j) continue;

      for (int k = 0; k < max_node_index_; ++k) {
        if (i == k || j == k) continue;
        auto other_path = dist_[i][k] + dist_[k][j];
        if (dist_[i][j] > other_path) {
          dist_[i][j] = other_path;
          prev_[i][j] = prev_[k][j];
        }
      }
    }
  }
  PLOGV << "Leaving FloydWarshall";
}
std::vector<AuxGraph::Node> AuxGraph::Path(Node s, Node t) {
  std::vector<Node> res;
  if (prev_[NodeToIndex(s)][NodeToIndex(t)] == -1) {
    return res;
  }

  res.emplace_back(t);

  auto s_index = NodeToIndex(s);
  auto t_index = NodeToIndex(t);

  while (s_index != t_index) {
    t_index = prev_[s_index][t_index];
    res.emplace_back(IndexToNode(t_index));
  }

  std::reverse(res.begin(), res.end());
  return res;

}
double AuxGraph::getWeight(AuxGraph::Node s, AuxGraph::Node t) {
  auto s_index = NodeToIndex(s);
  auto t_index = NodeToIndex(t);
  return dist_[s_index][t_index];
}
std::pair<double, std::vector<Node>> AuxGraph::BellmanFord(Node s, Node t) {
  // TODO
}
