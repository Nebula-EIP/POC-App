#include "node_base.hpp"

#include <iostream>
#include <algorithm>

core::NodeBase::Connection::Connection(NodeBase *n, uint8_t p, PinDataType t)
    : node(n), pin(p), type(t) {}

bool core::NodeBase::Connection::IsConnected() const { return node != nullptr; }

core::NodeBase::NodeBase(uint32_t id, NodeKind kind, std::pair<float, float> position)
    : id_(id), kind_(kind), position_(position) {}

uint32_t core::NodeBase::id() const { return id_; }

core::NodeBase::NodeKind core::NodeBase::kind() const { return kind_; }

std::pair<float, float> core::NodeBase::GetPosition() const {
    return position_;
}

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
    if (parent == nullptr
        || GetInputPinCount() <= in_pin
        || parent->GetOutputPinCount() <= out_pin) {
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
    if (child == nullptr
        || GetOutputPinCount() <= out_pin
        || child->GetInputPinCount() <= child_pin) {
        return;
    }

    // Make sure the vector is big enough to use out_pin as an index
    if (childrens_.size() <= out_pin) {
        childrens_.resize(out_pin + 1);
    }

    if (childrens_[out_pin].size() < childrens_[out_pin].capacity()) {
        // Inserts in the first empty space found
        auto it = std::find_if(childrens_[out_pin].begin(),
                            childrens_[out_pin].end(),
                            [](Connection conn) {return !conn.IsConnected();});
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
    if (child == nullptr
        || GetOutputPinCount() <= out_pin
        || child->GetInputPinCount() <= in_pin) {
        return;
    }

    auto &children = childrens_[out_pin];
    for (auto &connection : children) {
        if (connection.node == child &&
            connection.pin == in_pin) {
            connection.node = nullptr;
            connection.pin = 0;
            connection.type = PinDataType::kUndefined;
            break;
        }
    }
}

void core::NodeBase::Draw() {
    const auto [r, g, b] = color_;
    Color color = {r, g, b, 255};
    // Draw Node body
    DrawRectangle(position_.first, position_.second, 100, 50, color);
    // Draw Node number
    DrawText(("Node " + std::to_string(id_)).c_str(), position_.first + 10,
            position_.second + 15, 10, BLACK);
    // Draw Node kind
    DrawText(("Kind: " + std::to_string(static_cast<int>(kind_))).c_str(),
            position_.first + 10, position_.second + 30, 10, BLACK);
    // Draw pin
    for (uint8_t i = 0; i < GetInputPinCount(); i++) {
        DrawCircle(position_.first, position_.second + 25 + i * 15, 5, RED);
    }
    for (uint8_t i = 0; i < GetOutputPinCount(); i++) {
        DrawCircle(position_.first + 100, position_.second + 25 + i * 15, 5, BLUE);
    }
}

void core::NodeBase::PrepareDrag() {
    Vector2 cursorPosition = GetMousePosition();
    drag_offset_.first = 0;
    drag_offset_.second = 0;
    initial_position_cursor_.first = cursorPosition.x;
    initial_position_cursor_.second = cursorPosition.y;
    initial_position_ = position_;
}

void core::NodeBase::ClickNode() {
    Vector2 cursorPosition = GetMousePosition();
    if (CheckCollisionPointRec(cursorPosition, {position_.first, position_.second, 100, 50})) {
        color_ = {0.0, 255.0, 0.0}; // Change color when hovering
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            follow_mouse_ = !follow_mouse_;
            PrepareDrag();
        }
    } else {
        color_ = initial_color_; // Default color
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            follow_mouse_ = false;
        }
    }
}


void core::NodeBase::MoveNode() {
    Vector2 cursorPosition = GetMousePosition();
    if (follow_mouse_) {
        color_ = {0.0, 0.0, 255.0}; // Change color when following mouse
        drag_offset_.first = cursorPosition.x - initial_position_cursor_.first;
        drag_offset_.second = cursorPosition.y - initial_position_cursor_.second;
        position_.first = initial_position_.first + drag_offset_.first;
        position_.second = initial_position_.second + drag_offset_.second;
    }
}

bool core::NodeBase::IsMouseOver() const {
    Vector2 cursorPosition = GetMousePosition();

    return CheckCollisionPointRec(cursorPosition, {position_.first, position_.second, 100, 50});
}

void core::NodeBase::SetColor(unsigned char r, unsigned char g, unsigned char b) {
    color_ = {r, g, b};
}

std::tuple<unsigned char, unsigned char, unsigned char> core::NodeBase::GetInitialColor() const {
    return initial_color_;
}