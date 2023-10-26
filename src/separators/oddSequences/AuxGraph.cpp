//
// Created by max on 04.10.23.
//

#include <queue>
#include <algorithm>
#include "AuxGraph.h"
#include "progressbar.hpp"

void HalfChordedAuxGraph::AddArc(Node start, Node target, double weight) {
  int start_index = NodeToIndex(start);
  int target_index = NodeToIndex(target);

  if (start_index > max_node_index_ || target_index > max_node_index_ || start_index == target_index) {
    PLOGW << "Could not add [" << start_index << ", " << target_index << ", " << weight << "] to auxiliary graph.";
    return;
  }
  weights_[start_index].emplace_back(std::make_pair(weight, target_index));
}

// Implementation adopted from https://github.com/TheAlgorithms/C-Plus-Plus/blob/master/graph/dijkstra.cpp
std::pair<double, std::vector<AuxGraph::Node>> HalfChordedAuxGraph::shortestPath(Node start,
                                                                                 Node target) {
  int start_index = NodeToIndex(start);
  int target_index = NodeToIndex(target);

  std::vector<Node> path;

  if (start_index == target_index) {
    return std::make_pair(0, path);
  }

  auto dist = std::vector(max_node_index_ + 1, std::numeric_limits<double>::infinity());
  dist[start_index] = 0;
  auto pred = std::vector(max_node_index_ + 1, -1);
  pred[start_index] = start_index;

  /// creating a min heap using priority queue
  /// first element of pair contains the distance
  /// second element of pair contains the vertex
  std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> queue{};
  queue.emplace(0.0, start_index);

  while (!queue.empty()) {
    /// second element of pair denotes the node / vertex
    int current_node = queue.top().second;

    /// first element of pair denotes the distance
    double current_dist = queue.top().first;
    queue.pop();

    // since we're not actually interested in any shortest path, but rather a shortest path with length < 3
    // we can terminate this greedy search early
    if (current_dist > 3) {
      continue;
    }

    /// for all the reachable vertex from the currently exploring vertex
    /// we will try to minimize the distance
    for (auto [edge_weight, edge_target] : weights_[current_node]) {
      /// minimizing distances
      if (current_dist + edge_weight + 1e-9 < dist[edge_target]) {
        dist[edge_target] = current_dist + edge_weight;
        pred[edge_target] = current_node;
        queue.emplace(dist[edge_target], edge_target);
      }
    }
  }
  if (pred[target_index] == -1) {
    return std::make_pair(std::numeric_limits<double>::infinity(), path);
  }
  // reconstruct path
  auto current_node = target_index;
  while (current_node != start_index) {
    path.emplace_back(IndexToNode(current_node));
    current_node = pred[current_node];
  }
  path.emplace_back(start);
  std::reverse(path.begin(), path.end());
  return std::make_pair(dist[target_index], path);
}

void TwoChordedAuxGraph::AddArc(AuxGraph::Node start, AuxGraph::Node target, double weight) {
  int start_index = NodeToIndex(start);
  int target_index = NodeToIndex(target);

  if (start_index > max_node_index_ || target_index > max_node_index_ || start_index == target_index) {
    PLOGW << "Could not add [" << start_index << ", " << target_index << ", " << weight << "] to auxiliary graph.";
    return;
  }
  weights_[start_index][target_index] = weight;
  present_arcs_[start_index].emplace_back(target_index);
}
void TwoChordedAuxGraph::FloydWarshall() {
  PLOGV << "Starting FloydWarshall algorithm on auxiliary graph.";
  for (int i = 0; i < max_node_index_; ++i) {
    dist_[i][i] = 0;
    pred_[i][i] = -1;
    for (auto edge_target : present_arcs_[i]) {
      dist_[i][edge_target] = weights_[i][edge_target];
      pred_[i][edge_target] = i;
    }
  }

  progressbar bar(max_node_index_);

  for (int k = 0; k < max_node_index_; ++k) {
    if (plog::get()->getMaxSeverity() == plog::Severity::debug) {
      bar.update();
    }
    for (int i = 0; i < max_node_index_; ++i) {
      for (int j = 0; j < max_node_index_; ++j) {
        auto new_weight = dist_[i][k] + dist_[k][j];
        if (dist_[i][j] > new_weight + 1e-9) {
          dist_[i][j] = new_weight;
          pred_[i][j] = pred_[k][j];
        }
      }
    }
  }
  floyd_warshall_computed_ = true;
  PLOGV << "Finished FloydWarshall algorithm on auxiliary graph.";
}
std::pair<double, std::vector<AuxGraph::Node>> TwoChordedAuxGraph::shortestPath(AuxGraph::Node start,
                                                                                AuxGraph::Node target) {

  if (!floyd_warshall_computed_) {
    FloydWarshall();
  }

  int start_index = NodeToIndex(start);
  int target_index = NodeToIndex(target);

  std::vector<Node> path{};

  if (start_index == target_index) {
    return std::make_pair(0, path);
  }
  if (pred_[start_index][target_index] == -1) {
    return std::make_pair(std::numeric_limits<double>::infinity(), path);
  }


  // reconstruct path
  auto current_node = target_index;
  while (current_node != start_index) {
    path.emplace_back(IndexToNode(current_node));
    current_node = pred_[start_index][current_node];
  }
  path.emplace_back(start);
  std::reverse(path.begin(), path.end());
  return std::make_pair(dist_[start_index][target_index], path);
}
