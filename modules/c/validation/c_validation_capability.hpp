/**
 * @file c_validation_capability.hpp
 * @brief Defines the graph validation capability for the C module.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-11-2026
 */

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "graph/datatypes.hpp"
#include "graph/graph.hpp"
#include "graph/node.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"
#include "modules/capabilities/validation_capability.hpp"

namespace c_module {

/**
 * @brief Graph validation capability for the C module.
 *
 * Verifies that every node in the graph is supported by the C module, has valid
 * property values, compatible connected pin types, and all required input
 * connections before C/C++ code generation.
 */
class CValidationCapability final : public core::capa::IValidationCapability {
   public:
    explicit CValidationCapability(
        const core::capa::ITypeListCapability *types_capa = nullptr,
        const core::capa::INodeListCapability *nodes_capa = nullptr);
    ~CValidationCapability() override = default;

    /**
     * @brief Validates a graph against the C module's node and type rules.
     *
     * @param graph The graph to validate.
     * @return ValidationResult containing all detected blocking errors.
     */
    [[nodiscard]] core::capa::ValidationResult Validate(
        const core::Graph &graph) const override;

   private:
    const core::capa::ITypeListCapability *types_capa_;
    const core::capa::INodeListCapability *nodes_capa_;

    [[nodiscard]] std::vector<core::capa::ValidationError>
    CheckSupportedNodesAndValues(
        const core::Graph &graph,
        std::unordered_map<core::NodeId, std::string> &node_kinds) const;

    void CheckNodeProperties(
        const core::Graph &graph, const core::Node &node, std::string_view kind,
        const std::string &label,
        std::vector<core::capa::ValidationError> &errors) const;

    static void CheckIdentifierProperty(
        const core::Node &node, std::string_view kind, const std::string &label,
        std::vector<core::capa::ValidationError> &errors);

    [[nodiscard]] static std::string ResolveAndCheckTypeProperty(
        const core::Node &node, std::string_view kind, const std::string &label,
        std::vector<core::capa::ValidationError> &errors);

    static void CheckLiteralProperty(
        const core::Node &node, std::string_view value_type,
        const std::string &label,
        std::vector<core::capa::ValidationError> &errors);

    void CheckOperatorProperties(
        const core::Graph &graph, const core::Node &node,
        std::string_view value_type, const std::string &label,
        std::vector<core::capa::ValidationError> &errors) const;

    void CheckDivisionByZero(
        const core::Graph &graph, const core::Node &node,
        const std::string &label,
        std::vector<core::capa::ValidationError> &errors) const;

    static void CheckConditionProperty(
        const core::Node &node, const std::string &label,
        std::vector<core::capa::ValidationError> &errors);

    [[nodiscard]] std::vector<core::capa::ValidationError>
    CheckPinCompatibility(
        const core::Graph &graph,
        const std::unordered_map<core::NodeId, std::string> &node_kinds) const;

    void CheckSingleConnection(
        const core::Graph &graph, core::ConnectionId conn_id,
        const std::unordered_map<core::NodeId, std::string> &node_kinds,
        std::vector<core::capa::ValidationError> &errors) const;

    [[nodiscard]] bool VerifyPinTypesAgainstModuleConfig(
        const core::Node &from_node, std::string_view from_kind,
        const core::Pin &out_pin, const core::Node &to_node,
        std::string_view to_kind, const core::Pin &in_pin,
        std::string &out_type_name, std::string &in_type_name) const;

    [[nodiscard]] std::vector<core::capa::ValidationError> CheckPinConnectivity(
        const core::Graph &graph,
        const std::unordered_map<core::NodeId, std::string> &node_kinds) const;

    void CheckNodePinConnectivity(
        const core::Graph &graph, const core::Node &node, std::string_view kind,
        const std::string &label,
        std::vector<core::capa::ValidationError> &errors) const;

    [[nodiscard]] std::string ResolveNodeKind(const core::Node &node) const;

    [[nodiscard]] static std::string FormatNodeLabel(const core::Node &node,
                                                     std::string_view kind);

    [[nodiscard]] std::string ResolveTypeName(core::DataType type) const;
};

}  // namespace c_module
