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
    ~Graph();

    uint32_t AddNode(NodeBase::NodeKind kind);
    void RemoveNode(NodeBase *node);

    std::expected<void, std::string> Link(NodeBase *from, uint8_t out_pin, NodeBase *to, uint8_t in_pin);
    std::expected<void, std::string> Unlink(NodeBase *from, uint8_t out_pin, NodeBase *to, uint8_t in_pin);

 private:
    std::vector<std::unique_ptr<NodeBase>> nodes_;
};

} // namespace core
