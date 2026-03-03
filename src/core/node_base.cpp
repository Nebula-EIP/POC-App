#include "node_base.hpp"

#include <algorithm>

core::NodeBase::Connection::Connection(NodeBase *n, uint8_t p)
    : node(n), pin(p) {}

bool core::NodeBase::Connection::IsConnected() const { return node != nullptr; }

core::NodeBase::NodeBase(uint32_t id, NodeKind kind) : id_(id), kind_(kind) {}

uint32_t core::NodeBase::id() const { return id_; }

core::NodeBase::NodeKind core::NodeBase::kind() const { return kind_; }

// parents_ vector already filled by the Graph class
core::NodeBase::Connection *core::NodeBase::parent(uint8_t in_pin) const {
    return parents_[in_pin];
}

// childrens_ vector already filled by the Graph class
const std::vector<core::NodeBase::Connection *> &core::NodeBase::childrens(
    uint8_t out_pin) const {
    return childrens_[out_pin];
}

void core::NodeBase::SetParent(uint8_t in_pin, NodeBase *parent,
                               uint8_t parent_pin) {
    parents_[in_pin]->node = parent;
    parents_[in_pin]->pin = parent_pin;
    parents_[in_pin]->type = parent->GetOutputPinType(parent_pin);
}

void core::NodeBase::AddChild(uint8_t out_pin, NodeBase *child,
                              uint8_t child_pin) {
    if (childrens_[out_pin].size() < childrens_[out_pin].capacity()) {
        // Inserts in the first empty space found
        auto it = std::find(childrens_[out_pin].begin(),
                            childrens_[out_pin].end(), nullptr);
        if (it != childrens_[out_pin].end()) {
            (*it)->node = child;
            (*it)->pin = child_pin;
            (*it)->type = child->GetOutputPinType(child_pin);
        }
    }
}

void core::NodeBase::ClearParent(uint8_t pin) {
    parents_[pin]->node = nullptr;
    parents_[pin]->pin = 0;
    parents_[pin]->type = PinDataType::kUndefined;
}

void core::NodeBase::RemoveChild(uint8_t out_pin, NodeBase *node,
                                 uint8_t in_pin) {
    auto &children = childrens_[out_pin];
    for (auto &connection : children) {
        if (connection && connection->node == node &&
            connection->pin == in_pin) {
            connection->node = nullptr;
            connection->pin = 0;
            connection->type = PinDataType::kUndefined;
            break;
        }
    }
}
