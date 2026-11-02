/**
 * @file c_node_list_capability.cpp
 * @brief Implementation of the C module node list capability.
 *
 * @author Created by mathys-f
 * @date Created on 21-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 29-10-2026
 */

#include "c_node_list_capability.hpp"

#include <any>
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
        pending_descriptions_[next_node_index_],
        pending_categories_[next_node_index_]
    });
    
    const std::string_view *name_ptr = &pending_names_views_[next_node_index_];
    next_node_index_++;
    
    return name_ptr;
}

std::vector<core::capa::NodeMetadata> CNodeListCapability::GetAvailableNodes() const noexcept {
    std::vector<core::capa::NodeMetadata> metadata;
    for (const auto &node : registered_nodes_) {
        metadata.push_back({node.id_, node.name_, node.description_, node.category_});
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

std::string CNodeListCapability::NodeName(core::NodeType type) const {
    for (const auto &node : registered_nodes_) {
        if (node.id_ == type) {
            return node.name_;
        }
    }
    return {};
}

core::capa::NodeConfiguration CNodeListCapability::ResolveNodeConfiguration(
    core::NodeType type, const core::PropertyMap &properties) const {
    core::capa::NodeConfiguration config = GetNodeConfiguration(type);
    const std::string kName = NodeName(type);

    const auto kReadType = [this, &properties](std::string fallback) {
        const auto kProperty = properties.find(properties::kType);
        if (kProperty != properties.end()) {
            const auto *kText =
                std::any_cast<std::string>(&kProperty->second.value_);
            if (kText != nullptr && GetTypeId(*kText) != 0 &&
                *kText != "void") {
                return *kText;
            }
        }
        return fallback;
    };

    if (kName == "Variable") {
        const core::DataType kDataType = GetTypeId(kReadType("int"));
        for (auto &pin : config.input_pins_) {
            pin.type_ = kDataType;
        }
        for (auto &pin : config.output_pins_) {
            pin.type_ = kDataType;
        }
        return config;
    }

    if (kName == "Literal") {
        const core::DataType kDataType = GetTypeId(kReadType("int"));
        for (auto &pin : config.output_pins_) {
            pin.type_ = kDataType;
        }
        return config;
    }

    if (kName == "Operator") {
        const core::DataType kOperandType = GetTypeId(kReadType("int"));
        std::string op = "+";
        const auto kOpProperty = properties.find(properties::kOperator);
        if (kOpProperty != properties.end()) {
            const auto *kOpText =
                std::any_cast<std::string>(&kOpProperty->second.value_);
            if (kOpText != nullptr) {
                op = *kOpText;
            }
        }
        const bool kIsLogical = (op == "&&" || op == "||");
        const bool kIsComparison =
            (op == "==" || op == "!=" || op == "<" || op == "<=" ||
             op == ">" || op == ">=");
        const core::DataType kInputType =
            kIsLogical ? GetTypeId("bool") : kOperandType;
        const core::DataType kOutputType =
            (kIsLogical || kIsComparison) ? GetTypeId("bool") : kOperandType;
        for (auto &pin : config.input_pins_) {
            pin.type_ = kInputType;
        }
        for (auto &pin : config.output_pins_) {
            pin.type_ = kOutputType;
        }
        return config;
    }

    if (kName == "FunctionInput") {
        if (config.output_pins_.size() > 1) {
            config.output_pins_[1].type_ = GetTypeId(kReadType("int"));
        }
        return config;
    }

    if (kName == "FunctionOutput") {
        if (config.input_pins_.size() > 1) {
            config.input_pins_[1].type_ = GetTypeId(kReadType("int"));
        }
        return config;
    }

    if (kName == "Print") {
        if (config.input_pins_.size() > 1) {
            config.input_pins_[1].type_ = GetTypeId(kReadType("string"));
        }
        return config;
    }

    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetNodeConfiguration(core::NodeType type) const {
    std::string name = NodeName(type);
    
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
    config.input_pins_.push_back({1, "in", GetTypeId("int")});
    config.output_pins_.push_back({1, "out", GetTypeId("int")});
    config.default_properties_[properties::kName] = {properties::kName, std::string("variable")};
    config.default_properties_[properties::kType] = {properties::kType, std::string("int")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetLiteralConfig() const {
    core::capa::NodeConfiguration config;
    config.output_pins_.push_back({1, "out", GetTypeId("int")});
    config.default_properties_[properties::kValue] = {properties::kValue, 0.0};
    config.default_properties_[properties::kType] = {properties::kType, std::string("int")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetOperatorConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "left", GetTypeId("int")});
    config.input_pins_.push_back({2, "right", GetTypeId("int")});
    config.output_pins_.push_back({1, "result", GetTypeId("int")});
    config.default_properties_[properties::kOperator] = {properties::kOperator, std::string("+")};
    config.default_properties_[properties::kType] = {properties::kType, std::string("int")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetConditionConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "in", GetTypeId("void")});
    config.input_pins_.push_back({2, "cond", GetTypeId("bool")});
    config.output_pins_.push_back({1, "true", GetTypeId("void")});
    config.output_pins_.push_back({2, "false", GetTypeId("void")});
    config.output_pins_.push_back({3, "out", GetTypeId("void")});
    config.default_properties_[properties::kCondition] = {properties::kCondition, false};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetLoopConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "in", GetTypeId("void")});
    config.input_pins_.push_back({2, "cond", GetTypeId("bool")});
    config.output_pins_.push_back({1, "body", GetTypeId("void")});
    config.output_pins_.push_back({2, "exit", GetTypeId("void")});
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
    config.output_pins_.push_back({2, "out", GetTypeId("int")});
    config.default_properties_[properties::kName] = {properties::kName, std::string("param")};
    config.default_properties_[properties::kType] = {properties::kType, std::string("int")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetFunctionOutputConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "flow", GetTypeId("void")});
    config.input_pins_.push_back({2, "value", GetTypeId("int")});
    config.default_properties_[properties::kType] = {properties::kType, std::string("int")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetPrintConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "control", GetTypeId("void")});
    config.input_pins_.push_back({2, "value", GetTypeId("string")});
    config.output_pins_.push_back({1, "out", GetTypeId("void")});
    config.default_properties_[properties::kType] = {properties::kType, std::string("string")};
    return config;
}

core::capa::NodeConfiguration CNodeListCapability::GetForConfig() const {
    core::capa::NodeConfiguration config;
    config.input_pins_.push_back({1, "in", GetTypeId("void")});
    config.input_pins_.push_back({2, "init", GetTypeId("int")});
    config.input_pins_.push_back({3, "cond", GetTypeId("bool")});
    config.input_pins_.push_back({4, "step", GetTypeId("int")});
    config.output_pins_.push_back({1, "body", GetTypeId("void")});
    config.output_pins_.push_back({2, "index", GetTypeId("int")});
    config.output_pins_.push_back({3, "exit", GetTypeId("void")});
    config.default_properties_[properties::kName] = {properties::kName, std::string("i")};
    return config;
}

}  // namespace c_module
