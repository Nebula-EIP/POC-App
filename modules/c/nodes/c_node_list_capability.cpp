/**
 * @file c_node_list_capability.cpp
 * @brief Implementation of the C module node list capability.
 *
 * @author Created by mathys-f
 * @date Created on 21-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-09-2026
 */

#include "c_node_list_capability.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

#include "exception/graph_exception/node_exception.hpp"

namespace c_module {

CNodeListCapability::CNodeListCapability(core::capa::ITypeListCapability *types_capa)
    : types_capa_(types_capa) {
    for (const auto& name : pending_names_) {
        pending_names_views_.push_back(name);
    }
}

const std::string_view *CNodeListCapability::RegisterNode(core::NodeType node_type) const noexcept {
    if (next_node_index_ >= pending_names_views_.size()) {
        return nullptr;
    }
    
    registered_nodes_.push_back({
        node_type, 
        pending_names_[next_node_index_], 
        pending_descriptions_[next_node_index_]
    });
    
    const std::string_view *name_ptr = &pending_names_views_[next_node_index_];
    next_node_index_++;
    
    return name_ptr;
}

std::vector<core::capa::NodeMetadata> CNodeListCapability::GetAvailableNodes() const noexcept {
    std::vector<core::capa::NodeMetadata> metadata;
    for (const auto &node : registered_nodes_) {
        metadata.push_back({node.id_, node.name_, node.description_});
    }
    return metadata;
}

void CNodeListCapability::InitializePropertyTypes(
    const std::unordered_map<std::string, core::PropertyTypeId> &property_types) {
    property_types_ = property_types;
}

core::DataType CNodeListCapability::GetTypeId(std::string_view name) const {
    if (types_capa_) {
        return types_capa_->TypeId(name);
    }
    return 0; // Fallback
}

core::capa::NodeConfiguration CNodeListCapability::GetNodeConfiguration(core::NodeType type) const {
    // Find the node by type
    std::string name;
    for (const auto &node : registered_nodes_) {
        if (node.id_ == type) {
            name = node.name_;
            break;
        }
    }
    
    if (name.empty()) {
        throw core::NodeTypeException("Unknown node type requested.");
    }
    
    if (name == "Variable") return GetVariableConfig();
    if (name == "Literal") return GetLiteralConfig();
    if (name == "Operator") return GetOperatorConfig();
    if (name == "Condition") return GetConditionConfig();
    if (name == "Loop") return GetLoopConfig();
    if (name == "Function") return GetFunctionConfig();
    if (name == "FunctionInput") return GetFunctionInputConfig();
    if (name == "FunctionOutput") return GetFunctionOutputConfig();
    if (name == "Print") return GetPrintConfig();
    if (name == "For") return GetForConfig();
    
    throw core::NodeTypeException("Configuration not implemented for node: " + name);
}

core::capa::NodeConfiguration CNodeListCapability::GetVariableConfig() const {
    core::capa::NodeConfiguration config;
    config.output_pins_.push_back({1, "out", GetTypeId("int")});
    config.default_properties_[properties::kName] = {properties::kName, std::string("")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetLiteralConfig() const {
    core::capa::NodeConfiguration config;
    config.output_pins_.push_back({1, "out", GetTypeId("int")});
    config.default_properties_[properties::kValue] = {properties::kValue, 0.0};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetOperatorConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "left", GetTypeId("int")});
    config.input_pins_.push_back({2, "right", GetTypeId("int")});
    config.output_pins_.push_back({1, "result", GetTypeId("int")});
    config.default_properties_[properties::kOperator] = {properties::kOperator, std::string("+")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetConditionConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "cond", GetTypeId("bool")});
    config.output_pins_.push_back({1, "true", GetTypeId("void")});
    config.output_pins_.push_back({2, "false", GetTypeId("void")});
    config.default_properties_[properties::kCondition] = {properties::kCondition, false};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetLoopConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "cond", GetTypeId("bool")});
    config.output_pins_.push_back({1, "body", GetTypeId("void")});
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetFunctionConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "in", GetTypeId("void")});
    config.output_pins_.push_back({1, "out", GetTypeId("void")});
    config.default_properties_[properties::kName] = {properties::kName, std::string("my_function")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetFunctionInputConfig() const {
    core::capa::NodeConfiguration config;
    config.output_pins_.push_back({1, "flow", GetTypeId("void")});
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetFunctionOutputConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "flow", GetTypeId("void")});
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetPrintConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "control", GetTypeId("void")});
    config.input_pins_.push_back({2, "value", GetTypeId("string")});
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetForConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "init", GetTypeId("int")});
    config.input_pins_.push_back({2, "cond", GetTypeId("bool")});
    config.input_pins_.push_back({3, "step", GetTypeId("int")});
    config.output_pins_.push_back({1, "body", GetTypeId("void")});
    return config;
}

}  // namespace c_module
