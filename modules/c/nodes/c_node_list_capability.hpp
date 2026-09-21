/**
 * @file c_node_list_capability.hpp
 * @brief Defines the node list capability for the C module.
 *
 * @author Created by mathys-f
 * @date Created on 21-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-09-2026
 */

#pragma once

#include <unordered_map>
#include <vector>

#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"

namespace c_module {

/**
 * @brief Property identifiers used by the C module nodes.
 */
namespace properties {
    inline constexpr core::PropertyId kName = 1;
    inline constexpr core::PropertyId kValue = 2;
    inline constexpr core::PropertyId kOperator = 3;
    inline constexpr core::PropertyId kCondition = 4;
    inline constexpr core::PropertyId kType = 5;
}

/**
 * @brief Node list capability for the C module.
 * Provides the node catalogue, pins, and default properties for C nodes.
 */
class CNodeListCapability : public core::capa::INodeListCapability {
   public:
    explicit CNodeListCapability(core::capa::ITypeListCapability *types_capa);
    ~CNodeListCapability() override = default;

    const std::string_view *RegisterNode(core::NodeType node_type) const noexcept override;

    std::vector<core::capa::NodeMetadata> GetAvailableNodes() const noexcept override;

    void InitializePropertyTypes(
        const std::unordered_map<std::string, core::PropertyTypeId> &property_types) override;

    core::capa::NodeConfiguration GetNodeConfiguration(core::NodeType type) const override;

   private:
    struct RegisteredNode {
        core::NodeType id_;
        std::string name_;
        std::string description_;
    };

    core::capa::ITypeListCapability *types_capa_;
    
    // Nodes are listed in this order to match expected IDs if hardcoded elsewhere.
    // 1. Variable (Declaration?)
    // 2. Literal
    // 3. Operator (BinaryExpression?)
    // 4. Condition (Conditional?)
    // 5. Loop
    // 6. Function
    // 7. FunctionInput
    // 8. FunctionOutput
    // 9. Print
    // 10. For
    std::vector<std::string> pending_names_ = {
        "Variable",
        "Literal",
        "Operator",
        "Condition",
        "Loop",
        "Function",
        "FunctionInput",
        "FunctionOutput",
        "Print",
        "For"
    };

    std::vector<std::string> pending_descriptions_ = {
        "Variable declaration",
        "Literal value",
        "Mathematical or logical operator",
        "If/Else condition",
        "While loop",
        "Function declaration",
        "Function input parameters",
        "Function output return",
        "Print to console",
        "For loop"
    };

    mutable std::size_t next_node_index_ = 0;
    mutable std::vector<RegisteredNode> registered_nodes_;
    mutable std::vector<std::string_view> pending_names_views_;

    std::unordered_map<std::string, core::PropertyTypeId> property_types_;
    
    core::capa::NodeConfiguration GetVariableConfig() const;
    core::capa::NodeConfiguration GetLiteralConfig() const;
    core::capa::NodeConfiguration GetOperatorConfig() const;
    core::capa::NodeConfiguration GetConditionConfig() const;
    core::capa::NodeConfiguration GetLoopConfig() const;
    core::capa::NodeConfiguration GetFunctionConfig() const;
    core::capa::NodeConfiguration GetFunctionInputConfig() const;
    core::capa::NodeConfiguration GetFunctionOutputConfig() const;
    core::capa::NodeConfiguration GetPrintConfig() const;
    core::capa::NodeConfiguration GetForConfig() const;
    
    core::DataType GetTypeId(std::string_view name) const;
};

}  // namespace c_module
