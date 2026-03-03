#include "graph.hpp"

#include <algorithm>

using namespace core;

NodeBase* Graph::AddNode(NodeBase::NodeKind kind) {
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

void Graph::RemoveNode(NodeBase *node) {
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
        auto children = node->childrens(i);

        // Copy to avoid iterator invalidation
        std::vector<NodeBase::Connection*> children_copy(children.begin(), children.end());
        for (auto *child_conn : children_copy) {
            Unlink(node, i, child_conn->node, child_conn->pin);
        }
    }

    // Remove from nodes vector
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
        [node](const std::unique_ptr<NodeBase> &n) {
            return n.get() == node;
        });
    
    if (it != nodes_.end()) {
        nodes_.erase(it);
    }
}

NodeBase* Graph::GetNode(uint32_t id) const {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
        [id](const std::unique_ptr<NodeBase>& node) {
            return node->id() == id;
        });
    
    return it != nodes_.end() ? it->get() : nullptr;
}

std::expected<void, std::string> core::Graph::Link(NodeBase *from, uint8_t out_pin, NodeBase *to, uint8_t in_pin) {
    if (!from || !to) {
        return std::unexpected("Invalid node pointers (nullptr)");
    }

    if (out_pin >= from->GetOutputPinCount()) {
        return std::unexpected("Invalid output pin");
    }

    if (in_pin >= to->GetInputPinCount()) {
        return std::unexpected("Invalid input pin");
    }
    
    if (!from->CanConnectTo(out_pin, to, in_pin)) {
        return std::unexpected("Impossible connection, come back in later versions for more infos :p");
    }

    to->SetParent(in_pin, from, out_pin);
    from->AddChild(out_pin, to, in_pin);

    return {};
}

std::expected<void, std::string> core::Graph::Unlink(NodeBase *from, uint8_t out_pin, NodeBase *to, uint8_t in_pin) {
    if (!from || !to) {
        return std::unexpected("Invalid node pointers");
    }

    to->ClearParent(in_pin);
    from->RemoveChild(out_pin, to, in_pin);

    return {};
}

std::unique_ptr<NodeBase> Graph::CreateNode(uint32_t id, NodeBase::NodeKind kind) {
    switch (kind) {
        case NodeBase::NodeKind::kLiteral:
            // return std::make_unique<LiteralNode>(id);
        
        case NodeBase::NodeKind::kVariable:
            // return std::make_unique<VariableNode>(id);
        
        case NodeBase::NodeKind::kOperator:
            // return std::make_unique<OperatorNode>(id);
        
        case NodeBase::NodeKind::kFunction:
            // return std::make_unique<FunctionNode>(id);
        
        case NodeBase::NodeKind::kFunctionInput:
            // return std::make_unique<FunctionInputNode>(id);
        
        case NodeBase::NodeKind::kFunctionOutput:
            // return std::make_unique<FunctionOutputNode>(id);
        
        case NodeBase::NodeKind::kCondition:
            // return std::make_unique<ConditionNode>(id);
        
        case NodeBase::NodeKind::kLoop:
            // return std::make_unique<LoopNode>(id);
        
        case NodeBase::NodeKind::kUndefined:
        default:
            return nullptr;
    }
}
