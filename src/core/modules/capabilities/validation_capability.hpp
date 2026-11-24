/**
 * @file validation_capability.hpp
 * @brief Defines the capability interface for a module to validate a graph
 * before code generation or compilation.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 24-11-2026
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "graph/datatypes.hpp"
#include "graph/graph.hpp"
#include "modules/icapability.hpp"

namespace core::capa {

/**
 * @brief Identifies a specific pin on a node involved in a validation error.
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
        kIncompatiblePinTypes,      ///< Connected pin types do not match
        kIncompatibleValue,         ///< Node property value is invalid
        kUnsupportedNode,           ///< Node type cannot be exported
        kMissingRequiredInputPin,   ///< Required input pin is not connected
        kMissingRequiredOutputPin,  ///< Required output pin is not connected
        kInvalidPinIndex            ///< Connection references missing pin
    };

    ErrorType type_ = ErrorType::kIncompatiblePinTypes;
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
    [[nodiscard]] bool HasTypeError() const noexcept;
    [[nodiscard]] bool HasValueError() const noexcept;
    [[nodiscard]] bool HasUnsupportedNodeError() const noexcept;
    [[nodiscard]] bool HasPinConnectivityError() const noexcept;
};

/**
 * @brief Optional module capability that validates a graph against the module's
 * node and type rules before code export or compilation.
 */
class IValidationCapability : public core::ICapability {
   public:
    ~IValidationCapability() override = default;

    /**
     * @brief Store the data types assigned by the core to this module.
     * @param types Map of core-assigned data type IDs to owned type names.
     * The core must provide only types declared by the owning module.
     * Each call replaces the previously stored list with a local copy.
     */
    virtual void InitializeTypes(
        const std::unordered_map<DataType, std::string> &types) = 0;

    /**
     * @brief Store the node types assigned by the core to this module.
     * @param node_types Map of core-assigned node type IDs to owned names.
     * The core must provide only node types declared by the owning module.
     * Each call replaces the previously stored list with a local copy.
     */
    virtual void InitializeNodeTypes(
        const std::unordered_map<NodeType, std::string> &node_types) = 0;

    /**
     * @brief Validates the given graph against the module's rules.
     *
     * @param graph The graph to validate.
     * @return ValidationResult containing all detected blocking errors.
     */
    [[nodiscard]] virtual ValidationResult Validate(
        const Graph &graph) const = 0;
};

}  // namespace core::capa
