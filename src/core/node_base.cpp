#include "node_base.hpp"

#include <algorithm>

using namespace core;

core::NodeBase::Connection::Connection(NodeBase *n, uint8_t p)
    : node(n), pin(p) {}

bool core::NodeBase::Connection::IsConnected() const {
    return node != nullptr;
}

NodeBase::NodeBase(uint32_t id, NodeKind kind)
    : id_(id), kind_(kind) {}

uint32_t NodeBase::id() const {
    return id_;
}

NodeBase::NodeKind NodeBase::kind() const {
    return kind_;
}

// parents_ vector already filled by the Graph class
NodeBase::Connection *NodeBase::parent(uint8_t in_pin) const {
    return parents_[in_pin];
}

// childrens_ vector already filled by the Graph class
const std::vector<NodeBase::Connection *> &NodeBase::childrens(uint8_t out_pin) const {
    return childrens_[out_pin];
}

void NodeBase::SetParent(uint8_t in_pin, NodeBase *node, uint8_t parent_pin) {
    parents_[in_pin]->node = node;
    parents_[in_pin]->pin = parent_pin;
}

void NodeBase::AddChild(uint8_t out_pin, NodeBase *node, uint8_t child_pin) {
    if (childrens_[out_pin].size() < childrens_[out_pin].capacity()) {
        auto it = std::find(childrens_[out_pin].begin(), childrens_[out_pin].end(), nullptr);
        if (it != childrens_[out_pin].end()) {
            (*it)->node = node;
            (*it)->pin = child_pin;
        }
    }
}

void NodeBase::ClearParent(uint8_t pin) {
    parents_[pin]->node = nullptr;
    parents_[pin]->pin = pin;
}

void NodeBase::RemoveChild(uint8_t out_pin, NodeBase *node, uint8_t in_pin) {
    auto &children = childrens_[out_pin];
    for (auto &connection : children) {
        if (connection && connection->node == node && connection->pin == in_pin) {
            connection->node = nullptr;
            connection->pin = 0;
            break;
        }
    }
}
