/**
 * @file renderer_capability.cpp
 * @brief Implementation of the RendererCapability class.
 *
 * This file contains the implementation of the RendererCapability class,
 * which provides functionality for registering and retrieving custom
 * rendering components for different node types in the system.
 *
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 04-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 04-09-2026
 */

#include "renderer_capability.hpp"

#include <string>
#include <utility>

#include "../../exception/graph_exception/node_exception.hpp"

namespace core::capa {

void RendererCapability::RegisterNodeRenderer(NodeType node_type,
                                              ComponentProvider provider) {
    if (!provider) {
        throw InvalidNodeException(
            "Cannot register an empty renderer component provider");
    }

    const bool kInserted =
        providers_.emplace(node_type, std::move(provider)).second;
    if (!kInserted) {
        throw NodeAlreadyExistsException("A renderer for node type " +
                                         std::to_string(node_type) +
                                         " is already registered");
    }
}

bool RendererCapability::UnregisterNodeRenderer(NodeType node_type) noexcept {
    return providers_.erase(node_type) != 0;
}

std::size_t RendererCapability::RegisteredNodeTypeCount() const noexcept {
    return providers_.size();
}

bool RendererCapability::SupportsNodeType(NodeType node_type) const noexcept {
    return providers_.contains(node_type);
}

ComponentList RendererCapability::GetNodeComponents(
    NodeId node_id, NodeType node_type, const PropertyMap &properties) {
    const auto kProvider = providers_.find(node_type);
    if (kProvider == providers_.end()) {
        throw NodeTypeException("No renderer is registered for node type " +
                                std::to_string(node_type));
    }

    return kProvider->second(node_id, node_type, properties);
}

}  // namespace core::capa
