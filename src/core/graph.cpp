#include "graph.hpp"

#include <algorithm>
#include <format>
#include <iostream>

#include "nodes/literal_node.hpp"
#include "nodes/variable_node.hpp"

core::NodeBase *core::Graph::AddNode(NodeBase::NodeKind kind, std::pair<float, float> position) {
    if (kind == NodeBase::NodeKind::kUndefined) {
        return nullptr;
    }

    uint32_t id = next_id_++;
    auto node = CreateNode(id, kind, position);

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
    uint32_t id, NodeBase::NodeKind kind, std::pair<float, float> position) {
    switch (kind) {
        case NodeBase::NodeKind::kLiteral:
            return std::unique_ptr<LiteralNode>(new LiteralNode(id, kind, position));

        case NodeBase::NodeKind::kVariable:
            return std::unique_ptr<VariableNode>(new VariableNode(id, kind, position));

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

void core::Graph::Draw() {
    // Draw nodes
    for (const auto &node : nodes_) {
        node->Draw();
    }
    // Draw connections
    for (const auto &node : nodes_) {
        for (uint8_t i = 0; i < node->GetOutputPinCount(); i++) {
            const auto &childrens = node->childrens(i);
            for (const auto &child_conn : childrens) {
                Vector2 start = {node->GetPosition().first + 100,
                                 node->GetPosition().second + 25 + i * 15};
                Vector2 end = {child_conn.node->GetPosition().first,
                               child_conn.node->GetPosition().second +
                                   25 + child_conn.pin * 15};
                DrawLineBezier(start, end, 3, BLACK);
            }
        }
    }
}

void core::Graph::CheckNodeMovement() {
    for (const auto &node : nodes_) {
        node->ClickNode();
        node->MoveNode();
    }
}

void core::Graph::LinkingWithMouse() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        for (const auto &node : nodes_) {
            if (node->IsMouseOver()) {
                if (linking_from_node_ == nullptr) {
                    linking_from_node_ = node.get();
                } else {
                    if (node.get() == linking_from_node_) {
                        linking_from_node_ = nullptr;
                        break;
                    }
                    auto res = Link(linking_from_node_, 0, node.get(), 0);
                    if (!res) {
                        std::cerr << "Failed to link nodes: " << res.error() << std::endl;
                    }
                    linking_from_node_ = nullptr;
                }
                break;
            }
        }
    }

    if (linking_from_node_) {
        Vector2 cursorPos = GetMousePosition();
        DrawLineEx({linking_from_node_->GetPosition().first + 100, linking_from_node_->GetPosition().second + 25},
                   cursorPos, 2, GRAY);
    }
}

void core::Graph::SelectWithMouse() {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (!is_selecting_) {
            Vector2 start = GetMousePosition();
            selection_start_ = {start.x, start.y};
            is_selecting_ = true;
        }
    } else {
        if (is_selecting_){
            //Select the nodes within the selection rectangle
            Vector2 end = GetMousePosition();
            for (const auto &node : nodes_) {
                Vector2 nodePos = {node->GetPosition().first, node->GetPosition().second};

                float left   = std::min(selection_start_.first, end.x);
                float top    = std::min(selection_start_.second, end.y);
                float right  = std::max(selection_start_.first, end.x);
                float bottom = std::max(selection_start_.second, end.y);

                Rectangle selectionRect = {left, top, right - left, bottom - top};
                if (CheckCollisionPointRec(nodePos, selectionRect)) {
                    node->follow_mouse_ = true;
                    node->PrepareDrag();
                } else {
                    std::tuple<unsigned char, unsigned char, unsigned char> init_color = node->GetInitialColor();
                    node->SetColor(std::get<0>(init_color), std::get<1>(init_color), std::get<2>(init_color));
                }
            }
        }
        is_selecting_ = false;
    }

    //Draw the square and color the nodes
    if (is_selecting_) {
        Vector2 current = GetMousePosition();
        DrawRectangleLines(selection_start_.first, selection_start_.second,
                           current.x - selection_start_.first, current.y - selection_start_.second, GRAY);

        for (const auto &node : nodes_) {
            Vector2 nodePos = {node->GetPosition().first, node->GetPosition().second};
            
            float left   = std::min(selection_start_.first, current.x);
            float top    = std::min(selection_start_.second, current.y);
            float right  = std::max(selection_start_.first, current.x);
            float bottom = std::max(selection_start_.second, current.y);

            
            Rectangle selectionRect = {left, top, right - left, bottom - top};
            if (CheckCollisionPointRec(nodePos, selectionRect)) {
                node->SetColor(255, 255, 0);
            } else {
                std::tuple<unsigned char, unsigned char, unsigned char> init_color = node->GetInitialColor();
                node->SetColor(std::get<0>(init_color), std::get<1>(init_color), std::get<2>(init_color));
            }
        }
    }
}