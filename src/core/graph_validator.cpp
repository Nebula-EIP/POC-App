#include "graph_validator.hpp"

#include <algorithm>
#include <sstream>

#include "graph.hpp"
#include "logger.hpp"
#include "node_base.hpp"

namespace core {

// ============================================================================
// ValidationResult helper methods
// ============================================================================

bool GraphValidator::ValidationResult::HasCycleError() const {
    return std::any_of(
        errors.begin(), errors.end(), [](const ValidationError &e) {
            return e.type == ValidationError::ErrorType::kCycleDetected;
        });
}

bool GraphValidator::ValidationResult::HasTypeError() const {
    return std::any_of(
        errors.begin(), errors.end(), [](const ValidationError &e) {
            return e.type == ValidationError::ErrorType::kIncompatiblePinTypes;
        });
}

bool GraphValidator::ValidationResult::HasPinConnectivityError() const {
    return std::any_of(
        errors.begin(), errors.end(), [](const ValidationError &e) {
            return e.type ==
                       ValidationError::ErrorType::kMissingRequiredInputPin ||
                   e.type ==
                       ValidationError::ErrorType::kMissingRequiredOutputPin;
        });
}

// ============================================================================
// Main Validate method
// ============================================================================

GraphValidator::ValidationResult GraphValidator::Validate(const Graph &graph) {
    ValidationResult result;
    result.is_valid = true;

    // Check 1: Detect cycles
    auto cycle_errors = DetectCycles(graph);
    result.errors.insert(result.errors.end(), cycle_errors.begin(),
                         cycle_errors.end());

    // Check 2: Pin type compatibility
    auto type_errors = CheckPinCompatibility(graph);
    result.errors.insert(result.errors.end(), type_errors.begin(),
                         type_errors.end());

    // Check 3: Pin connectivity
    auto connectivity_errors = CheckPinConnectivity(graph);
    result.errors.insert(result.errors.end(), connectivity_errors.begin(),
                         connectivity_errors.end());

    result.is_valid = result.errors.empty();
    return result;
}

// ============================================================================
// Cycle Detection - DFS Colored
// ============================================================================

std::vector<GraphValidator::ValidationError> GraphValidator::DetectCycles(
    const Graph &graph) {
    std::vector<ValidationError> errors;
    std::unordered_map<uint32_t, DFSColor> colors;

    // Initialize all nodes as WHITE
    const auto &all_nodes = graph.GetAllNodes();
    for (const auto &node : all_nodes) {
        if (node != nullptr) {
            colors[node->id()] = DFSColor::kWhite;
        }
    }

    // Visit each node
    for (const auto &node : all_nodes) {
        if (node != nullptr && colors[node->id()] == DFSColor::kWhite) {
            DFSVisit(node->id(), colors, node->id(), graph, errors);
        }
    }

    return errors;
}

bool GraphValidator::DFSVisit(uint32_t node_id,
                              std::unordered_map<uint32_t, DFSColor> &colors,
                              uint32_t parent_id, const Graph &graph,
                              std::vector<ValidationError> &errors) {
    // Mark as GRAY (currently processing)
    colors[node_id] = DFSColor::kGray;

    NodeBase *current_node = FindNodeById(graph, node_id);
    if (current_node == nullptr) {
        return false;
    }

    // Process all outgoing edges
    const auto &all_children = current_node->GetAllChildrens();
    for (const auto &child_conn : all_children) {
        if (!child_conn.IsConnected() || child_conn.node == nullptr) {
            continue;
        }

        uint32_t child_id = child_conn.node->id();

        if (colors[child_id] == DFSColor::kGray) {
            // Back edge found = cycle detected
            ValidationError error;
            error.type = ValidationError::ErrorType::kCycleDetected;

            std::ostringstream oss;
            oss << "Cycle detected: node " << node_id << " (pin "
                << child_conn.out_pin << ") → node " << child_id << " (pin "
                << child_conn.in_pin << ")";
            error.message = oss.str();

            error.involved_node_ids.push_back(node_id);
            error.involved_node_ids.push_back(child_id);
            error.involved_pins.push_back({node_id, child_conn.out_pin});
            error.involved_pins.push_back({child_id, child_conn.in_pin});

            errors.push_back(error);
            return true;
        } else if (colors[child_id] == DFSColor::kWhite) {
            // Recursively visit
            if (DFSVisit(child_id, colors, node_id, graph, errors)) {
                return true;
            }
        }
        // If kBlack, already processed, skip
    }

    // Mark as BLACK (fully processed)
    colors[node_id] = DFSColor::kBlack;
    return false;
}

// ============================================================================
// Pin Type Compatibility Check
// ============================================================================

std::vector<GraphValidator::ValidationError>
GraphValidator::CheckPinCompatibility(const Graph &graph) {
    std::vector<ValidationError> errors;

    const auto &all_nodes = graph.GetAllNodes();
    for (const auto &node : all_nodes) {
        if (node == nullptr) {
            continue;
        }

        // Check all outgoing connections from this node
        const auto &all_children = node->GetAllChildrens();
        for (const auto &child_conn : all_children) {
            if (!child_conn.IsConnected() || child_conn.node == nullptr) {
                continue;
            }

            // Get source and target pin types
            auto source_type = node->GetOutputPinType(child_conn.out_pin);
            auto target_type =
                child_conn.node->GetInputPinType(child_conn.in_pin);

            // Check compatibility
            if (!AreTypesCompatible(static_cast<int>(source_type),
                                    static_cast<int>(target_type))) {
                ValidationError error;
                error.type = ValidationError::ErrorType::kIncompatiblePinTypes;

                std::ostringstream oss;
                oss << "Pin type mismatch: "
                    << PinTypeToString(static_cast<int>(source_type))
                    << " (node " << node->id() << ", pin "
                    << static_cast<int>(child_conn.out_pin) << ") → "
                    << PinTypeToString(static_cast<int>(target_type))
                    << " (node " << child_conn.node->id() << ", pin "
                    << static_cast<int>(child_conn.in_pin) << ")";
                error.message = oss.str();

                error.involved_node_ids.push_back(node->id());
                error.involved_node_ids.push_back(child_conn.node->id());
                error.involved_pins.push_back({node->id(), child_conn.out_pin});
                error.involved_pins.push_back(
                    {child_conn.node->id(), child_conn.in_pin});

                errors.push_back(error);
            }
        }
    }

    return errors;
}

bool GraphValidator::AreTypesCompatible(int source_type_int,
                                        int target_type_int) {
    auto source_type = static_cast<NodeBase::PinDataType>(source_type_int);
    auto target_type = static_cast<NodeBase::PinDataType>(target_type_int);
    // Exact match
    if (source_type == target_type) {
        return true;
    }

    // Implicit conversions
    // int → float (promotion)
    if (source_type == NodeBase::PinDataType::kInt &&
        target_type == NodeBase::PinDataType::kFloat) {
        return true;
    }

    // void is compatible with anything (for control flow)
    if (source_type == NodeBase::PinDataType::kVoid ||
        target_type == NodeBase::PinDataType::kVoid) {
        return true;
    }

    // No other implicit conversions allowed
    return false;
}

std::string GraphValidator::PinTypeToString(int type_int) {
    auto type = static_cast<NodeBase::PinDataType>(type_int);
    switch (type) {
        case NodeBase::PinDataType::kInt:
            return "int";
        case NodeBase::PinDataType::kFloat:
            return "float";
        case NodeBase::PinDataType::kBool:
            return "bool";
        case NodeBase::PinDataType::kString:
            return "std::string";
        case NodeBase::PinDataType::kVoid:
            return "void";
        default:
            return "undefined";
    }
}

// ============================================================================
// Pin Connectivity Check
// ============================================================================

std::vector<GraphValidator::ValidationError>
GraphValidator::CheckPinConnectivity(const Graph &graph) {
    std::vector<ValidationError> errors;

    const auto &all_nodes = graph.GetAllNodes();
    for (const auto &node : all_nodes) {
        if (node == nullptr) {
            continue;
        }

        // Check all input pins
        uint8_t input_count = node->GetInputPinCount();
        for (uint8_t pin = 0; pin < input_count; ++pin) {
            // Note: For now, we check all input pins. Some nodes may have
            // optional inputs, but that's a more advanced feature.
            if (!node->IsInputPinConnected(pin)) {
                // Check if this node type requires this pin to be connected
                // For now, we only check for nodes that explicitly require
                // connections (e.g., OperatorNode, but not FunctionInputNode
                // which can be unconstrained)

                auto kind = node->kind();
                bool requires_connection = false;

                // Define which node types require all input pins to be
                // connected
                if (kind == NodeBase::NodeKind::kOperator ||
                    kind == NodeBase::NodeKind::kFunction) {
                    requires_connection = true;
                }

                if (requires_connection) {
                    ValidationError error;
                    error.type =
                        ValidationError::ErrorType::kMissingRequiredInputPin;

                    std::ostringstream oss;
                    oss << "Required input pin not connected: node "
                        << node->id() << ", pin " << static_cast<int>(pin)
                        << " (" << node->GetInputPinName(pin) << ")";
                    error.message = oss.str();

                    error.involved_node_ids.push_back(node->id());
                    error.involved_pins.push_back({node->id(), pin});

                    errors.push_back(error);
                }
            }
        }

        // Check all output pins (most nodes require outputs to be used)
        uint8_t output_count = node->GetOutputPinCount();
        for (uint8_t pin = 0; pin < output_count; ++pin) {
            if (!node->IsOutputPinConnected(pin)) {
                // Note: In many cases, unused outputs are acceptable (dead
                // code) Only enforce this for nodes where outputs are required
                // to be used
                auto kind = node->kind();
                bool requires_usage = false;

                // Typically only function output nodes require usage
                if (kind == NodeBase::NodeKind::kFunctionOutput) {
                    requires_usage = true;
                }

                if (requires_usage) {
                    ValidationError error;
                    error.type =
                        ValidationError::ErrorType::kMissingRequiredOutputPin;

                    std::ostringstream oss;
                    oss << "Output pin should be used: node " << node->id()
                        << ", pin " << static_cast<int>(pin) << " ("
                        << node->GetOutputPinName(pin) << ")";
                    error.message = oss.str();

                    error.involved_node_ids.push_back(node->id());
                    error.involved_pins.push_back({node->id(), pin});

                    errors.push_back(error);
                }
            }
        }
    }

    return errors;
}

// ============================================================================
// Helper Methods
// ============================================================================

NodeBase *GraphValidator::FindNodeById(const Graph &graph, uint32_t id) {
    return graph.GetNode(id);
}

}  // namespace core
