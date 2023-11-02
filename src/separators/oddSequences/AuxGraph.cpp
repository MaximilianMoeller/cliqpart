//
// Created by max on 04.10.23.
//

#include <queue>
#include <algorithm>
#include <chrono>
#include "AuxGraph.h"
#include "progressbar.hpp"

void AuxGraph::AddArc(AuxGraph::Node start, AuxGraph::Node target, double weight) {
  int start_index = NodeToIndex(start);
  int target_index = NodeToIndex(target);

  if (start_index > max_node_index_ || target_index > max_node_index_ || start_index == target_index) {
    PLOGW << "Could not add [" << start_index << ", " << target_index << ", " << weight << "] to auxiliary graph.";
    return;
  }
  weights_[start_index].emplace_back(std::make_pair(weight, target_index));
}

// Implementation adopted from https://github.com/TheAlgorithms/C-Plus-Plus/blob/master/graph/dijkstra.cpp
std::pair<double, std::vector<AuxGraph::Node>> HalfChordedAuxGraph::ShortestPath(Node start,
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

std::pair<double, std::vector<AuxGraph::Node>> TwoChordedAuxGraph::ShortestPath(Node start,
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

  bool progress{false};
  // repeat outer loop |V| - 1 times
  // |V| = max_node_index + 1 (because 0 is a valid index)
  for (int i = 0; i < max_node_index_; ++i) {
    // if nothing changes within a single loop, nothing will change in all subsequent iterations
    progress = false;
    for (int edge_source = 0; edge_source < max_node_index_ + 1; ++edge_source) {
      for (auto [edge_weight, edge_target] : weights_[edge_source]) {
        if (dist[edge_source] + edge_weight + 1e-9 < dist[edge_target]) {
          dist[edge_target] = dist[edge_source] + edge_weight;
          pred[edge_target] = edge_source;
          progress = true;
        }
      }
    }
    if (!progress) break;
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
