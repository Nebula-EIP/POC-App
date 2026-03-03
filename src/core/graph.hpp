/**
 * This version of the Nebula Node Graph V0.
 * Purposely built to handle basic C without includes nor macros.
 */

#pragma once
#include <cstdint>
#include <memory>
#include <expected>
#include <vector>

#include "node_base.hpp"

namespace core {

class Graph {
 public:
    Graph() = default;
    ~Graph() = default;

    NodeBase* AddNode(NodeBase::NodeKind kind);

    template<typename T>
    T *AddNode(NodeBase::NodeKind kind);

    void RemoveNode(NodeBase *node);
    NodeBase *GetNode(uint32_t id) const;

    template<typename T>
    T *GetNode(uint32_t id) const;

    std::expected<void, std::string> Link(NodeBase *from, uint8_t out_pin, NodeBase *to, uint8_t in_pin);
    std::expected<void, std::string> Unlink(NodeBase *from, uint8_t out_pin, NodeBase *to, uint8_t in_pin);

 private:
    std::unique_ptr<NodeBase> CreateNode(uint32_t id, NodeBase::NodeKind kind);

    uint32_t next_id_ = 0;
    std::vector<std::unique_ptr<NodeBase>> nodes_;
};

} // namespace core

#include "graph.tcc"
