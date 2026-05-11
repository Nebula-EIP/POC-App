#include "topological_sorter.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "graph.hpp"
#include "node_base.hpp"

namespace core {

std::vector<NodeBase *> TopologicalSorter::Sort(const Graph &graph) {
    std::vector<NodeBase *> sorted_nodes;

    // Get all nodes
    const auto &all_nodes = graph.GetAllNodes();

    // Empty graph case
    if (all_nodes.empty()) {
        return sorted_nodes;
    }

    // Step 1: Calculate in-degree for each node
    std::unordered_map<uint32_t, uint32_t> in_degree;
    for (const auto &node : all_nodes) {
        in_degree[node->id()] = CalculateInDegree(node);
    }

    // Step 2: Initialize queue with nodes having in-degree 0
    std::queue<NodeBase *> zero_in_degree_queue;
    for (const auto &node : all_nodes) {
        if (in_degree[node->id()] == 0) {
            zero_in_degree_queue.push(node);
        }
    }

    // Step 3: Process queue
    while (!zero_in_degree_queue.empty()) {
        // Dequeue node with in-degree 0
        NodeBase *current = zero_in_degree_queue.front();
        zero_in_degree_queue.pop();

        // Add to sorted result
        sorted_nodes.push_back(current);

        // Get all neighbors (nodes this node points to)
        auto neighbors = GetNeighbors(current, graph);

        // For each neighbor, decrease in-degree
        for (NodeBase *neighbor : neighbors) {
            in_degree[neighbor->id()]--;

            // If neighbor's in-degree becomes 0, enqueue it
            if (in_degree[neighbor->id()] == 0) {
                zero_in_degree_queue.push(neighbor);
            }
        }
    }

    // Step 4: Verify all nodes were processed
    // If sorted_nodes.size() != all_nodes.size(), there's a cycle
    // (Should not happen after graph validation)
    if (sorted_nodes.size() != all_nodes.size()) {
        // Return empty vector to signal failure
        // In production, might throw an exception or log error
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
    std::unordered_set<uint32_t> seen_neighbors;  // To avoid duplicates

    if (!node) {
        return neighbors;
    }

    // For each output pin
    for (uint8_t out_pin = 0; out_pin < node->GetOutputPinCount(); ++out_pin) {
        // Get all children connected to this output pin
        const auto *children = node->Childrens(out_pin);

        if (!children) {
            continue;
        }

        for (const auto &child_conn : *children) {
            if (child_conn.IsConnected() && child_conn.node) {
                uint32_t neighbor_id = child_conn.node->id();

                // Add neighbor only if we haven't seen it yet
                if (seen_neighbors.find(neighbor_id) == seen_neighbors.end()) {
                    neighbors.push_back(child_conn.node);
                    seen_neighbors.insert(neighbor_id);
                }
            }
        }
    }

    return neighbors;
}

}  // namespace core
