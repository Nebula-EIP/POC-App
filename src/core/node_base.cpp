#include "node_base.hpp"

#include <algorithm>

core::NodeBase::Connection::Connection(NodeBase *n, uint8_t p)
    : node(n), pin(p) {}

bool core::NodeBase::Connection::IsConnected() const { return node != nullptr; }

core::NodeBase::NodeBase(uint32_t id, NodeKind kind) : id_(id), kind_(kind) {}

core::NodeBase::~NodeBase() {
    // Clean up parent connections
    for (auto* conn : parents_) {
        delete conn;
    }
    parents_.clear();

    // Clean up children connections
    for (auto& child_vec : childrens_) {
        for (auto* conn : child_vec) {
            delete conn;
        }
        child_vec.clear();
    }
    childrens_.clear();
}

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
    auto* conn = new Connection(child, child_pin);
    conn->type = GetOutputPinType(out_pin);
    childrens_[out_pin].push_back(conn);
}

void core::NodeBase::ClearParent(uint8_t pin) {
    parents_[pin]->node = nullptr;
    parents_[pin]->pin = 0;
    parents_[pin]->type = PinDataType::kUndefined;
}

void core::NodeBase::RemoveChild(uint8_t out_pin, NodeBase *node,
                                 uint8_t in_pin) {
    auto &children = childrens_[out_pin];
    auto it = std::find_if(children.begin(), children.end(),
                           [node, in_pin](Connection* conn) {
                               return conn && conn->node == node && conn->pin == in_pin;
                           });
    
    if (it != children.end()) {
        delete *it;
        children.erase(it);
    }
}

void core::NodeBase::InitializeConnections() {
    // Initialize parent connections (one per input pin)
    parents_.clear();
    parents_.reserve(GetInputPinCount());
    for (uint8_t i = 0; i < GetInputPinCount(); ++i) {
        parents_.push_back(new Connection());
    }

    // Initialize children connections (one vector per output pin)
    childrens_.clear();
    childrens_.reserve(GetOutputPinCount());
    for (uint8_t i = 0; i < GetOutputPinCount(); ++i) {
        childrens_.push_back(std::vector<Connection *>());
    }
}
