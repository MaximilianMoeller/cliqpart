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
  arcs_.emplace_back(NodeToIndex(s), NodeToIndex(t), weight);

}
std::pair<double, std::vector<AuxGraph::Node>> AuxGraph::BellmanFord(Node s, Node t) {
  std::vector<double> distance = std::vector(max_node_index_ + 1, std::numeric_limits<double>::infinity());
  std::vector<int> pred = std::vector(max_node_index_ + 1, -1);

  distance[NodeToIndex(s)] = 0;

  for (int counter = 0; counter < max_node_index_; counter++) {
    for (auto [u, v, weight] : arcs_) {
      if (distance[u] + weight + 1e-3 < distance[v]) {
        distance[v] = distance[u] + weight;
        pred[v] = u;
      }

    }
  }

  std::vector<Node> res;
  if (pred[NodeToIndex(t)] == -1) {
    return std::make_pair(INFINITY, res);
  }

  res.emplace_back(t);

  auto s_index = NodeToIndex(s);
  auto t_index = NodeToIndex(t);

  while (s_index != t_index) {
    t_index = pred[t_index];
    res.emplace_back(IndexToNode(t_index));
  }

  std::reverse(res.begin(), res.end());

  return std::make_pair(distance[NodeToIndex(t)], res);

}
