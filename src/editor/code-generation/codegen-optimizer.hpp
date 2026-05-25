#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/node_base.hpp"

namespace core {
class Graph;
}

namespace editor::code_generation {

struct TypeInfo {
    core::NodeBase::PinDataType type;
    bool is_constant = false;
    bool is_used = false;
};

struct DeadCodeAnalysis {
    std::unordered_set<uint32_t> dead_nodes;
    std::unordered_map<uint32_t, TypeInfo> type_info;
};

class CodegenOptimizer {
   public:
    CodegenOptimizer() = default;

    /**
     * @brief Analyzes the graph for dead code and infers types of node outputs.
     * @param graph The input graph to analyze
     * @return A DeadCodeAnalysis struct containing sets of dead nodes and type
     * information
     */
    DeadCodeAnalysis AnalyzeGraph(const core::Graph &graph);

    /**
     * @brief Infers the data types of node outputs based on their connections
     * and usage in the graph.
     * @param graph The input graph to analyze
     * @param type_map An output parameter that will be filled with node ID to
     * PinDataType mappings for each node's output pins
     */
    void InferTypes(
        const core::Graph &graph,
        std::unordered_map<uint32_t, core::NodeBase::PinDataType> &type_map);

    /**
     * @brief Finds all nodes that are used in the graph.
     * @param graph The input graph to analyze
     * @return A set of node IDs that are used in the graph
     */
    std::unordered_set<uint32_t> FindUsedNodes(const core::Graph &graph);

   private:
    /**
     * @brief Recursively marks nodes as used starting from output nodes and
     * following connections backwards.
     * @param node_id The ID of the node to mark as used
     * @param graph The graph context to find connections
     * @param used_nodes A set that accumulates the IDs of used nodes
     */
    void MarkAsUsed(uint32_t node_id, const core::Graph &graph,
                    std::unordered_set<uint32_t> &used_nodes);
};

}  // namespace editor::code_generation
