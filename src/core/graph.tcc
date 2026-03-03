#pragma once

#include "graph.hpp"

namespace core {

template <typename T>
T *Graph::AddNode(NodeBase::NodeKind kind) {
    static_assert(std::is_base_of_v<NodeBase, T>,
                  "T must derive from core::NodeBase");
    return static_cast<T *>(AddNode(kind));
}

template <typename T>
T *Graph::GetNode(uint32_t id) const {
    static_assert(std::is_base_of_v<NodeBase, T>,
                  "T must derive from core::NodeBase");
    return static_cast<T *>(GetNode(id));
}

}  // namespace core
