#include "graph.hpp"

#include <algorithm>
#include <format>
#include <iostream>

#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

core::NodeBase *core::Graph::AddNode(NodeBase::NodeKind kind) {
    if (kind == NodeBase::NodeKind::kUndefined) {
        return nullptr;
    }

    uint32_t id = next_id_++;
    auto node = CreateNode(id, kind);

    if (!node) {
        return nullptr;
    }

    nodes_.push_back(std::move(node));
    return nodes_.back().get();
}

void core::Graph::RemoveNode(NodeBase *node) {
    if (!node) return;

    // Disconnect all connections
    for (uint8_t i = 0; i < node->GetInputPinCount(); i++) {
        auto parent_conn = node->parent(i);
        if (parent_conn && parent_conn->IsConnected()) {
            Unlink(parent_conn->node, parent_conn->pin, node, i);
        }
    }

    // Same for outputs
    for (uint8_t i = 0; i < node->GetOutputPinCount(); i++) {
        const auto &childrens = node->childrens(i);

        // Copy to avoid iterator invalidation
        std::vector<NodeBase::Connection> childrens_copy(childrens.begin(),
                                                         childrens.end());
        for (auto &child_conn : childrens_copy) {
            Unlink(node, i, child_conn.node, child_conn.pin);
        }
    }

    // Remove from nodes vector
    auto it = std::find_if(
        nodes_.begin(), nodes_.end(),
        [node](const std::unique_ptr<NodeBase> &n) { return n.get() == node; });

    if (it != nodes_.end()) {
        nodes_.erase(it);
    }
}

core::NodeBase *core::Graph::GetNode(uint32_t id) const {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
                           [id](const std::unique_ptr<NodeBase> &node) {
                               return node->id() == id;
                           });

    return it != nodes_.end() ? it->get() : nullptr;
}

std::expected<void, std::string> core::Graph::Link(NodeBase *from,
                                                   uint8_t out_pin,
                                                   NodeBase *to,
                                                   uint8_t in_pin) {
    if (!from || !to) {
        return std::unexpected(std::format(
            "{} pointer is null", ((from == nullptr) ? "1st" : "2nd")));
    }

    if (out_pin >= from->GetOutputPinCount()) {
        return std::unexpected("Output pin out of bounds");
    }

    if (in_pin >= to->GetInputPinCount()) {
        return std::unexpected("Input pin out of bounds");
    }

    auto res = from->CanConnectTo(out_pin, to, in_pin);
    if (!res) {
        return std::unexpected("{}");
    }

    if (to->parent(in_pin)) {
        const NodeBase::Connection *conn = to->parent(in_pin);
        Unlink(conn->node, conn->pin, to, in_pin);
    }

    to->SetParent(in_pin, from, out_pin);
    from->AddChild(out_pin, to, in_pin);

    return {};
}

std::expected<void, std::string> core::Graph::Unlink(NodeBase *from,
                                                     uint8_t out_pin,
                                                     NodeBase *to,
                                                     uint8_t in_pin) {
    if (!from || !to) {
        return std::unexpected("Invalid node pointers");
    }

    to->ClearParent(in_pin);
    from->RemoveChild(out_pin, to, in_pin);

    return {};
}

std::unique_ptr<core::NodeBase> core::Graph::CreateNode(
    uint32_t id, NodeBase::NodeKind kind) {
    switch (kind) {
        case NodeBase::NodeKind::kLiteral:
            return std::unique_ptr<LiteralNode>(new LiteralNode(id, kind));

        case NodeBase::NodeKind::kVariable:
            return std::unique_ptr<VariableNode>(new VariableNode(id, kind));

        case NodeBase::NodeKind::kOperator:

        case NodeBase::NodeKind::kFunction:

        case NodeBase::NodeKind::kFunctionInput:

        case NodeBase::NodeKind::kFunctionOutput:

        case NodeBase::NodeKind::kCondition:

        case NodeBase::NodeKind::kLoop:

        case NodeBase::NodeKind::kUndefined:
        default:
            return nullptr;
    }
}
