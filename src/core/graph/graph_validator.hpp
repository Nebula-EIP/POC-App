/**
 * @file graph_validator.hpp
 * @brief Validates a graph before C++ code generation or compilation.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 15-11-2026
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "graph/datatypes.hpp"
#include "graph/graph.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"
#include "node_catalog/node_catalog.hpp"

namespace core {

/**
 * @brief Checks whether a graph is valid and can be converted into C++.
 *
 * Detects missing required connections, incompatible pin types or property
 * values, unsupported node types, and invalid circular dependencies while
 * permitting valid loop constructs (such as a For loop's index feeding into its
 * own condition or step expression).
 */
class GraphValidator {
   public:
    /**
     * @brief Identifies a specific pin on a node involved in a validation
     * error.
     */
    struct InvolvedPin {
        NodeId node_id_ = 0;
        PinId pin_id_ = 0;
        bool is_input_ = true;
    };

    /**
     * @brief Describes a single validation error found in the graph.
     */
    struct ValidationError {
        enum class ErrorType : std::uint8_t {
            kCycleDetected,             ///< Invalid circular dependency found
            kIncompatiblePinTypes,      ///< Connected pin types do not match
            kIncompatibleValue,         ///< Node property value is invalid
            kUnsupportedNode,           ///< Node type cannot be exported to C++
            kMissingRequiredInputPin,   ///< Required input pin is not connected
            kMissingRequiredOutputPin,  ///< Required output pin is not
                                        ///< connected
            kInvalidPinIndex            ///< Connection references missing pin
        };

        ErrorType type_ = ErrorType::kCycleDetected;
        std::string message_;
        std::vector<NodeId> involved_node_ids_;
        std::vector<InvolvedPin> involved_pins_;
    };

    /**
     * @brief Aggregated validation result for a graph.
     */
    struct ValidationResult {
        bool is_valid_ = true;
        std::vector<ValidationError> errors_;

        [[nodiscard]] std::size_t ErrorCount() const noexcept {
            return errors_.size();
        }
        [[nodiscard]] bool HasCycleError() const noexcept;
        [[nodiscard]] bool HasTypeError() const noexcept;
        [[nodiscard]] bool HasValueError() const noexcept;
        [[nodiscard]] bool HasUnsupportedNodeError() const noexcept;
        [[nodiscard]] bool HasPinConnectivityError() const noexcept;
    };

    /**
     * @brief Optional metadata resolvers for richer validation and messages.
     */
    struct ValidationContext {
        const NodeCatalog *catalog_ = nullptr;
        const capa::INodeListCapability *nodes_ = nullptr;
        const capa::ITypeListCapability *types_ = nullptr;
        std::function<std::string(NodeId)> node_title_;
        std::function<std::string(NodeType)> node_type_name_;
        std::function<std::string(DataType)> type_name_;
    };

    GraphValidator() = default;
    ~GraphValidator() = default;

    /**
     * @brief Validates a graph using default C module conventions.
     *
     * @param graph The graph to validate.
     * @return ValidationResult containing all detected errors.
     */
    [[nodiscard]] ValidationResult Validate(const Graph &graph) const;

    /**
     * @brief Validates a graph using the supplied module/catalog context.
     *
     * @param graph The graph to validate.
     * @param context Optional catalog, module capabilities, and label
     * resolvers.
     * @return ValidationResult containing all detected errors.
     */
    [[nodiscard]] ValidationResult Validate(
        const Graph &graph, const ValidationContext &context) const;

   private:
    enum class DFSColor : std::uint8_t { kWhite, kGray, kBlack };

    [[nodiscard]] std::vector<ValidationError> CheckSupportedNodesAndValues(
        const Graph &graph, const ValidationContext &context,
        std::unordered_map<NodeId, std::string> &node_kinds) const;

    [[nodiscard]] std::vector<ValidationError> CheckPinCompatibility(
        const Graph &graph, const ValidationContext &context,
        const std::unordered_map<NodeId, std::string> &node_kinds) const;

    [[nodiscard]] std::vector<ValidationError> CheckPinConnectivity(
        const Graph &graph, const ValidationContext &context,
        const std::unordered_map<NodeId, std::string> &node_kinds) const;

    [[nodiscard]] std::vector<ValidationError> DetectCycles(
        const Graph &graph, const ValidationContext &context,
        const std::unordered_map<NodeId, std::string> &node_kinds) const;

    [[nodiscard]] static std::string ResolveNodeKind(
        const Node &node, const ValidationContext &context);

    [[nodiscard]] static std::string FormatNodeLabel(
        const Node &node, const std::string &kind_name,
        const ValidationContext &context);

    [[nodiscard]] static std::string ResolveTypeName(
        DataType type, const ValidationContext &context);
};

}  // namespace core
