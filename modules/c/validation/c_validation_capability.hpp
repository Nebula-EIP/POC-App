/**
 * @file c_validation_capability.hpp
 * @brief Defines the graph validation capability for the C module.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 24-11-2026
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "graph/datatypes.hpp"
#include "graph/graph.hpp"
#include "graph/node.hpp"
#include "modules/capabilities/validation_capability.hpp"

namespace c_module {

/**
 * @brief Supported C/C++ value types in the C module.
 */
enum class CValueType : std::uint8_t {
    kVoid,
    kInt,
    kFloat,
    kDouble,
    kChar,
    kBool,
    kString,
};

/**
 * @brief Supported C/C++ binary operators in the C module.
 */
enum class COperator : std::uint8_t {
    kAdd,
    kSub,
    kMul,
    kDiv,
    kMod,
    kEq,
    kNe,
    kLt,
    kLe,
    kGt,
    kGe,
    kLogicalAnd,
    kLogicalOr,
    kBitAnd,
    kBitOr,
    kBitXor,
    kShiftLeft,
    kShiftRight,
};

/**
 * @brief Supported node kinds in the C module.
 */
enum class CNodeKind : std::uint8_t {
    kVariable,
    kLiteral,
    kOperator,
    kCondition,
    kLoop,
    kFunction,
    kFunctionInput,
    kFunctionOutput,
    kPrint,
    kFor,
};

/**
 * @brief Graph validation capability for the C module.
 *
 * Verifies that every node in the graph is supported by the C module, has valid
 * property values, compatible connected pin types, and all required input
 * connections before C/C++ code generation.
 */
class CValidationCapability final : public core::capa::IValidationCapability {
   public:
    CValidationCapability() = default;
    ~CValidationCapability() override = default;

    /**
     * @brief Replace the local copy of the owning module's data type list.
     */
    void InitializeTypes(
        const std::unordered_map<core::DataType, std::string> &types) override;

    /**
     * @brief Replace the local copy of the owning module's node type list.
     */
    void InitializeNodeTypes(
        const std::unordered_map<core::NodeType, std::string> &node_types)
        override;

    /**
     * @brief Validates a graph against the C module's node and type rules.
     *
     * @param graph The graph to validate.
     * @return ValidationResult containing all detected blocking errors.
     */
    [[nodiscard]] core::capa::ValidationResult Validate(
        const core::Graph &graph) const override;

   private:
    std::unordered_map<core::DataType, std::string> types_;
    std::unordered_map<core::NodeType, std::string> node_types_;

    [[nodiscard]] std::vector<core::capa::ValidationError>
    CheckSupportedNodesAndValues(
        const core::Graph &graph,
        std::unordered_map<core::NodeId, CNodeKind> &node_kinds) const;

    void CheckNodeProperties(
        const core::Graph &graph, const core::Node &node, CNodeKind kind,
        const std::string &label,
        std::vector<core::capa::ValidationError> &errors) const;

    static void CheckIdentifierProperty(
        const core::Node &node, CNodeKind kind, const std::string &label,
        std::vector<core::capa::ValidationError> &errors);

    [[nodiscard]] static CValueType ResolveAndCheckTypeProperty(
        const core::Node &node, CNodeKind kind, const std::string &label,
        std::vector<core::capa::ValidationError> &errors);

    static void CheckLiteralProperty(
        const core::Node &node, CValueType value_type, const std::string &label,
        std::vector<core::capa::ValidationError> &errors);

    void CheckOperatorProperties(
        const core::Graph &graph, const core::Node &node, CValueType value_type,
        const std::string &label,
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
        const std::unordered_map<core::NodeId, CNodeKind> &node_kinds) const;

    void CheckSingleConnection(
        const core::Graph &graph, core::ConnectionId conn_id,
        const std::unordered_map<core::NodeId, CNodeKind> &node_kinds,
        std::vector<core::capa::ValidationError> &errors) const;

    [[nodiscard]] std::vector<core::capa::ValidationError> CheckPinConnectivity(
        const core::Graph &graph,
        const std::unordered_map<core::NodeId, CNodeKind> &node_kinds) const;

    void CheckNodePinConnectivity(
        const core::Graph &graph, const core::Node &node, CNodeKind kind,
        const std::string &label,
        std::vector<core::capa::ValidationError> &errors) const;

    [[nodiscard]] std::optional<CNodeKind> ResolveNodeKind(
        const core::Node &node) const;

    [[nodiscard]] static std::string FormatNodeLabel(
        const core::Node &node, std::optional<CNodeKind> kind);

    [[nodiscard]] std::optional<CValueType> ResolveDataType(
        core::DataType type) const;

    [[nodiscard]] std::string ResolveTypeName(core::DataType type) const;
};

}  // namespace c_module
