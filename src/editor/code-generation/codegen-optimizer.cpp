#include "codegen-optimizer.hpp"

#include "core/graph.hpp"
#include "core/node_base.hpp"
#include "core/nodes/operator_node.hpp"
#include "core/nodes/literal_node.hpp"
#include <algorithm>

namespace editor::code_generation {

using PinDataType = core::NodeBase::PinDataType;
using OperatorType = core::OperatorNode::OperatorType;

// Helper: get the output type of a node
static PinDataType GetNodeOutputType(const core::NodeBase *node) {
    if (!node) return PinDataType::kUndefined;
    return node->GetOutputPinType(0);
}

// Helper: get input type for a pin
static PinDataType GetNodeInputType(const core::NodeBase *node, uint8_t pin) {
    if (!node) return PinDataType::kUndefined;
    return node->GetInputPinType(pin);
}

// Helper: promote types (int + float = float)
static PinDataType PromoteType(PinDataType lhs, PinDataType rhs) {
    // Float is highest type
    if (lhs == PinDataType::kFloat || rhs == PinDataType::kFloat) {
        return PinDataType::kFloat;
    }
    // String comparison
    if (lhs == PinDataType::kString || rhs == PinDataType::kString) {
        return PinDataType::kString;
    }
    // Bool/Int comparison
    if (lhs == PinDataType::kBool || rhs == PinDataType::kBool) {
        return PinDataType::kBool;
    }
    // Default to first type
    return lhs;
}

void CodegenOptimizer::MarkAsUsed(uint32_t node_id,
                                   const core::Graph &graph,
                                   std::unordered_set<uint32_t> &used_nodes) {
    if (used_nodes.count(node_id)) {
        return;  // Already processed
    }
    used_nodes.insert(node_id);
    
    // Mark all parents as used (they feed into this node)
    auto *node = graph.GetNode(node_id);
    if (node) {
        auto parents = node->GetAllParents();
        for (const auto &parent : parents) {
            if (parent.IsConnected() && parent.node != nullptr) {
                MarkAsUsed(parent.node->id(), graph, used_nodes);
            }
        }
    }
}

std::unordered_set<uint32_t> CodegenOptimizer::FindUsedNodes(const core::Graph &graph) {
    std::unordered_set<uint32_t> used_nodes;
    
    // Find all leaf nodes (nodes with no children or operator nodes at the end)
    const auto &all_nodes = graph.GetAllNodes();
    for (const auto &node_ptr : all_nodes) {
        const auto *node = node_ptr.get();
        auto children = node->GetAllChildrens();
        
        // If node has no children, it's a result node
        if (children.empty()) {
            MarkAsUsed(node->id(), graph, used_nodes);
        }
    }
    
    return used_nodes;
}

void CodegenOptimizer::InferTypes(const core::Graph &graph,
                                  std::unordered_map<uint32_t, PinDataType> &type_map) {
    const auto &all_nodes = graph.GetAllNodes();
    
    // Initialize with node's declared output types
    for (const auto &node_ptr : all_nodes) {
        const auto *node = node_ptr.get();
        type_map[node->id()] = GetNodeOutputType(node);
    }
    
    // For operators, infer output type from inputs
    for (const auto &node_ptr : all_nodes) {
        const auto *node = node_ptr.get();
        
        if (auto *op = dynamic_cast<const core::OperatorNode *>(node)) {
            auto parents = node->GetAllParents();
            
            if (op->IsUnaryOperator() && !parents.empty()) {
                // Unary: output type depends on operation and input type
                auto parent_type = type_map[parents[0].node->id()];
                
                // Logical not returns bool
                if (op->operator_type() == OperatorType::kLogicalNot) {
                    type_map[node->id()] = PinDataType::kBool;
                }
                // Bitwise not keeps input type
                else if (op->operator_type() == OperatorType::kBitwiseNot) {
                    type_map[node->id()] = parent_type;
                }
            } else if (!op->IsUnaryOperator() && parents.size() >= 2) {
                // Binary: infer from operands
                auto left_type = type_map[parents[0].node->id()];
                auto right_type = type_map[parents[1].node->id()];
                
                // Comparison operations return bool
                if (op->operator_type() == OperatorType::kEqual ||
                    op->operator_type() == OperatorType::kNotEqual ||
                    op->operator_type() == OperatorType::kLessThan ||
                    op->operator_type() == OperatorType::kGreaterThan ||
                    op->operator_type() == OperatorType::kLessOrEqual ||
                    op->operator_type() == OperatorType::kGreaterOrEqual) {
                    type_map[node->id()] = PinDataType::kBool;
                }
                // Logical operations return bool
                else if (op->operator_type() == OperatorType::kLogicalAnd ||
                         op->operator_type() == OperatorType::kLogicalOr) {
                    type_map[node->id()] = PinDataType::kBool;
                }
                // Arithmetic: promote types
                else {
                    type_map[node->id()] = PromoteType(left_type, right_type);
                }
            }
        }
    }
}

DeadCodeAnalysis CodegenOptimizer::AnalyzeGraph(const core::Graph &graph) {
    DeadCodeAnalysis analysis;
    
    // Find all used nodes
    auto used_nodes = FindUsedNodes(graph);
    
    // Infer types
    std::unordered_map<uint32_t, PinDataType> type_map;
    InferTypes(graph, type_map);
    
    // Mark dead nodes
    const auto &all_nodes = graph.GetAllNodes();
    for (const auto &node_ptr : all_nodes) {
        const auto *node = node_ptr.get();
        uint32_t node_id = node->id();
        
        // Check if node is used
        bool is_used = used_nodes.count(node_id) > 0;
        
        if (!is_used) {
            analysis.dead_nodes.insert(node_id);
        }
        
        // Store type info
        analysis.type_info[node_id] = TypeInfo{
            type_map[node_id],
            dynamic_cast<const core::LiteralNode *>(node) != nullptr,
            is_used
        };
    }
    
    return analysis;
}

}  // namespace editor::code_generation
