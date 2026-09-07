/**
 * @file node_list_capability.cpp
 * @brief Implementation of the node list capability for modules.
 *
 * @author Created by mathys-f
 * @date Created on 07-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 07-09-2026
 */

#include "node_list_capability.hpp"

#include "../../exception/graph_exception/node_exception.hpp"

namespace core::capa {

void NodeListCapability::RegisterNode(NodeType type, std::string name,
                                      std::string description,
                                      NodeConfiguration config) {
    if (node_configs_.find(type) != node_configs_.end()) {
        throw NodeAlreadyExistsException(
            "Node type is already registered in this capability.");
    }

    available_nodes_.push_back({type, std::move(name), std::move(description)});
    node_configs_[type] = std::move(config);
}

std::vector<NodeMetadata> NodeListCapability::GetAvailableNodes()
    const noexcept {
    return available_nodes_;
}

void NodeListCapability::InitializePropertyTypes(
    const std::unordered_map<std::string, PropertyTypeId> &property_types) {
    property_types_ = property_types;
}

NodeConfiguration NodeListCapability::GetNodeConfiguration(
    NodeType type) const {
    auto it = node_configs_.find(type);
    if (it == node_configs_.end()) {
        throw NodeTypeException(
            "Requested node type is not registered in this module.");
    }
    return it->second;
}

}  // namespace core::capa
