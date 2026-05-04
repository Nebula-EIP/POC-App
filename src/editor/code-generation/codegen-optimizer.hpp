#pragma once

#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <optional>

#include "core/node_base.hpp"

namespace core {
class Graph;
}

namespace editor::code_generation {

// Type information for a node
struct TypeInfo {
    core::NodeBase::PinDataType type;
    bool is_constant = false;
    bool is_used = false;  // Whether this value is actually used
};

// Results of dead code analysis
struct DeadCodeAnalysis {
    std::unordered_set<uint32_t> dead_nodes;  // Node IDs that are not used
    std::unordered_map<uint32_t, TypeInfo> type_info;  // Type info for each node
};

// Optimizer for graph computations
class CodegenOptimizer {
  public:
    CodegenOptimizer() = default;
    
    // Analyze graph for dead code and type information
    DeadCodeAnalysis AnalyzeGraph(const core::Graph &graph);
    
    // Infer types for all nodes in the graph
    void InferTypes(const core::Graph &graph, 
                   std::unordered_map<uint32_t, core::NodeBase::PinDataType> &type_map);
    
    // Find all nodes that contribute to the final result
    std::unordered_set<uint32_t> FindUsedNodes(const core::Graph &graph);
    
  private:
    // Helper: recursively mark nodes as used
    void MarkAsUsed(uint32_t node_id, 
                   const core::Graph &graph,
                   std::unordered_set<uint32_t> &used_nodes);
};

}  // namespace editor::code_generation
