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

void NodeListCapability::RegisterNode(std::string name, std::string description,
                                      NodeConfiguration config) {
    for (const auto &node : pending_nodes_) {
        if (node.name == name) {
            throw NodeAlreadyExistsException(
                "Node is already registered in this capability.");
        }
    }
    for (const auto &node : available_nodes_) {
        if (node.name_ == name) {
            throw NodeAlreadyExistsException(
                "Node is already registered in this capability.");
        }
    }

    pending_nodes_.push_back(
        {std::move(name), std::move(description), std::move(config)});
    pending_names_.push_back(pending_nodes_.back().name);
}

const std::string_view *NodeListCapability::registerNode(
    NodeType node_type) const noexcept {
    if (next_node_index_ >= pending_nodes_.size()) {
        return nullptr;
    }

    // Find the pending node by index
    auto it = pending_nodes_.begin();
    std::advance(it, next_node_index_);

    available_nodes_.push_back({node_type, it->name, it->description});
    node_configs_[node_type] = it->config;

    std::string_view *name_ptr = &pending_names_[next_node_index_];
    next_node_index_++;

    return name_ptr;
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
