#include "topological_sorter.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>

#include "graph.hpp"
#include "node_base.hpp"

namespace core {

std::vector<NodeBase *> TopologicalSorter::Sort(const Graph &graph) {
    std::vector<NodeBase *> sorted_nodes;
    const auto &all_nodes = graph.GetAllNodes();

    if (all_nodes.empty()) {
        return sorted_nodes;
    }

    // Step 1: Calculate in-degree for each node
    std::unordered_map<uint32_t, uint32_t> in_degree;
    for (const auto &node : all_nodes) {
        in_degree[node->id()] = CalculateInDegree(node.get());
    }

    // Step 2: Initialize queue with nodes having in-degree 0
    std::queue<NodeBase *> zero_in_degree_queue;
    for (const auto &node : all_nodes) {
        if (in_degree[node->id()] == 0) {
            zero_in_degree_queue.push(node.get());
        }
    }

    // Step 3: Process queue
    while (!zero_in_degree_queue.empty()) {
        NodeBase *current = zero_in_degree_queue.front();
        zero_in_degree_queue.pop();
        sorted_nodes.push_back(current);
        auto neighbors = GetNeighbors(current, graph);
        for (NodeBase *neighbor : neighbors) {
            in_degree[neighbor->id()]--;
            if (in_degree[neighbor->id()] == 0) {
                zero_in_degree_queue.push(neighbor);
            }
        }
    }

    // Step 4: Verify all nodes were processed
    if (sorted_nodes.size() != all_nodes.size()) {
        return std::vector<NodeBase *>();
    }

    return sorted_nodes;
}

uint32_t TopologicalSorter::CalculateInDegree(NodeBase *node) {
    if (!node) {
        return 0;
    }

    // In-degree = number of connected input pins
    uint32_t in_degree_count = 0;
    const auto &parents = node->GetAllParents();

    for (const auto &parent_conn : parents) {
        if (parent_conn.IsConnected()) {
            in_degree_count++;
        }
    }

    return in_degree_count;
}

std::vector<NodeBase *> TopologicalSorter::GetNeighbors(NodeBase *node,
                                                        const Graph &graph) {
    std::vector<NodeBase *> neighbors;

    if (!node) {
        return neighbors;
    }

    for (uint8_t out_pin = 0; out_pin < node->GetOutputPinCount(); ++out_pin) {
        // Get all children connected to this output pin
        const auto *children = node->Childrens(out_pin);

        if (!children) {
            continue;
        }

        for (const auto &child_conn : *children) {
            if (child_conn.IsConnected() && child_conn.node) {
                neighbors.push_back(child_conn.node);
            }
        }
    }

    return neighbors;
}

}  // namespace core
