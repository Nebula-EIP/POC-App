/**
 * @file node_list_capability.hpp
 * @brief Defines the capability for a module to provide custom nodes.
 *
 * @author Created by mathys-f
 * @date Created on 06-08-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 06-08-2026
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../../graph/datatypes.hpp"
#include "../icapability.hpp"

namespace core {
namespace capa {

/**
 * @brief Metadata describing a node provided by a module.
 */
struct NodeMetadata {
    NodeType type_;  ///< Unique identifier for the node type within the module.
    std::string name_;         ///< Human-readable name of the node.
    std::string description_;  ///< Short description of the node's behavior.
};

/**
 * @brief Configuration data required to build a Node.
 */
struct NodeConfiguration {
    std::vector<Pin> input_pins_;     ///< The input pins for the node.
    std::vector<Pin> output_pins_;    ///< The output pins for the node.
    PropertyMap default_properties_;  ///< The default properties the node
                                      ///< should be initialized with.
};

/**
 * @brief Interface for the capability providing custom nodes.
 */
class INodeListCapability : public ICapability {
   public:
    virtual ~INodeListCapability() = default;

    /**
     * @brief Queries the list of all nodes provided by this module.
     *
     * @return A list of metadata for all available nodes.
     */
    virtual std::vector<NodeMetadata> GetAvailableNodes() const noexcept = 0;

    /**
     * @brief Initializes the node list capability with the resolved property
     * types.
     *
     * @param property_types A map associating property names to their
     * corresponding PropertyTypeId. Required.
     */
    virtual void InitializePropertyTypes(
        const std::unordered_map<std::string, PropertyTypeId>
            &property_types) = 0;

    /**
     * @brief Retrieves the layout and default configuration for a specific node
     * type.
     *
     * @param type The NodeType identifier to get the configuration for.
     * Required.
     *
     * @return The configuration needed to build the specified node.
     */
    virtual NodeConfiguration GetNodeConfiguration(NodeType type) const = 0;
};

}  // namespace capa
}  // namespace core