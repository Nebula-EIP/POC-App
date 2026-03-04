#include "node_base.hpp"

#include <algorithm>

core::NodeBase::Connection::Connection(NodeBase *n, uint8_t p, PinDataType t)
    : node(n), pin(p), type(t) {}

bool core::NodeBase::Connection::IsConnected() const { return node != nullptr; }

core::NodeBase::NodeBase(uint32_t id, NodeKind kind) : id_(id), kind_(kind) {}

uint32_t core::NodeBase::id() const { return id_; }

core::NodeBase::NodeKind core::NodeBase::kind() const { return kind_; }

// parents_ vector already filled by the Graph class
const core::NodeBase::Connection *core::NodeBase::parent(uint8_t in_pin) const {
    if (GetInputPinCount() <= in_pin) {
        return nullptr;
    }

    const Connection &conn = parents_[in_pin];
    return conn.IsConnected() ? &conn : nullptr;
}

// childrens_ vector already filled by the Graph class
const std::vector<core::NodeBase::Connection> &core::NodeBase::childrens(
    uint8_t out_pin) const {
    static std::vector<NodeBase::Connection> empty_vector;

    if (GetOutputPinCount() <= out_pin) {
        return empty_vector;
    }

    return childrens_[out_pin];
}

void core::NodeBase::SetParent(uint8_t in_pin, NodeBase *parent,
                               uint8_t out_pin) {
    if (parent == nullptr || GetInputPinCount() <= in_pin ||
        parent->GetOutputPinCount() <= out_pin) {
        return;
    }

    // Make sure the vector is big enough to use in_pin as an index
    if (parents_.size() <= in_pin) {
        parents_.resize(out_pin + 1);
    }

    parents_[in_pin].node = parent;
    parents_[in_pin].pin = out_pin;
    parents_[in_pin].type = parent->GetOutputPinType(out_pin);
}

void core::NodeBase::AddChild(uint8_t out_pin, NodeBase *child,
                              uint8_t child_pin) {
    if (child == nullptr || GetOutputPinCount() <= out_pin ||
        child->GetInputPinCount() <= child_pin) {
        return;
    }

    // Make sure the vector is big enough to use out_pin as an index
    if (childrens_.size() <= out_pin) {
        childrens_.resize(out_pin + 1);
    }

    if (childrens_[out_pin].size() < childrens_[out_pin].capacity()) {
        // Inserts in the first empty space found
        auto it =
            std::find_if(childrens_[out_pin].begin(), childrens_[out_pin].end(),
                         [](Connection conn) { return !conn.IsConnected(); });
        if (it != childrens_[out_pin].end()) {
            it->node = child;
            it->pin = child_pin;
            it->type = child->GetOutputPinType(child_pin);
        }
    } else {
        // Add the new connection at the back of the vector
        Connection conn(child, child_pin, child->GetOutputPinType(child_pin));
        childrens_[out_pin].push_back(conn);
    }
}

void core::NodeBase::ClearParent(uint8_t pin) {
    if (GetInputPinCount() <= pin) {
        return;
    }

    parents_[pin].node = nullptr;
    parents_[pin].pin = 0;
    parents_[pin].type = PinDataType::kUndefined;
}

void core::NodeBase::RemoveChild(uint8_t out_pin, const NodeBase *child,
                                 uint8_t in_pin) {
    if (child == nullptr || GetOutputPinCount() <= out_pin ||
        child->GetInputPinCount() <= in_pin) {
        return;
    }

    auto &children = childrens_[out_pin];
    for (auto &connection : children) {
        if (connection.node == child && connection.pin == in_pin) {
            connection.node = nullptr;
            connection.pin = 0;
            connection.type = PinDataType::kUndefined;
            break;
        }
    }
}
