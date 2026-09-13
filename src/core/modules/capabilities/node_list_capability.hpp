/**
 * @file node_list_capability.hpp
 * @brief Defines the capability for a module to provide custom nodes.
 *
 * @author Created by mathys-f
 * @date Created on 06-08-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 07-09-2026
 */

#pragma once

#include <list>
#include <string>
#include <string_view>
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
     * @brief Used by the core to retrieve the nodes the capability wants to
     * export.
     *
     * @param node_type Available id the capability must assign to one of its
     * nodes.
     *
     * @return A pointer to a string_view with the new node name, nullptr if all
     * the nodes of the capability have been assigned.
     */
    virtual const std::string_view *RegisterNode(
        NodeType node_type) const noexcept = 0;

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

/**
 * @brief Reusable node list capability backed by registered nodes.
 *
 * Modules register their nodes with their configuration before the core assigns
 * IDs.
 */
class NodeListCapability final : public INodeListCapability {
   public:
    NodeListCapability() = default;
    ~NodeListCapability() override = default;

    /**
     * @brief Register a node for the module.
     *
     * @param name The human-readable name of the node.
     * @param description The description of the node.
     * @param config The configuration of the node.
     * @throws NodeAlreadyExistsException if a node with the same name is
     * already registered.
     */
    void RegisterNode(std::string name, std::string description,
                      NodeConfiguration config);

    const std::string_view *RegisterNode(
        NodeType node_type) const noexcept override;

    std::vector<NodeMetadata> GetAvailableNodes() const noexcept override;

    void InitializePropertyTypes(
        const std::unordered_map<std::string, PropertyTypeId> &property_types)
        override;

    NodeConfiguration GetNodeConfiguration(NodeType type) const override;

   private:
    struct PendingNode {
        std::string name_;
        std::string description_;
        NodeConfiguration config_;
    };

    std::list<PendingNode> pending_nodes_;
    mutable std::vector<std::string_view> pending_names_;
    mutable std::size_t next_node_index_ = 0;

    mutable std::vector<NodeMetadata> available_nodes_;
    mutable std::unordered_map<NodeType, NodeConfiguration> node_configs_;
    mutable std::unordered_map<std::string, PropertyTypeId> property_types_;
};

}  // namespace capa
}  // namespace core
