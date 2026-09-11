/**
 * @file node.hpp
 * @brief Definition of the Node class
 *
 * @author Created by JeanBizeul
 * @date Created on 08-08-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 08-08-2026
 */

#include "node.hpp"

#include <algorithm>

namespace core {

#pragma region Public

NodeId Node::Id() const noexcept { return id_; }

NodeType Node::Type() const noexcept { return type_; }

const std::vector<Pin> &Node::InputPins() const noexcept { return input_pins_; }

const std::vector<Pin> &Node::OutputPins() const noexcept {
    return output_pins_;
}

size_t Node::InputPinsCount() const noexcept { return input_pins_.size(); }

size_t Node::OutputPinsCount() const noexcept { return output_pins_.size(); }

bool Node::InputPinExists(PinId id) const noexcept {
    return std::find_if(input_pins_.begin(), input_pins_.end(),
                        [id](const Pin &pin) { return pin.id_ == id; }) !=
           input_pins_.end();
}

bool Node::OutputPinExists(PinId id) const noexcept {
    return std::find_if(output_pins_.begin(), output_pins_.end(),
                        [id](const Pin &pin) { return pin.id_ == id; }) !=
           output_pins_.end();
}

const Pin *Node::InputPin(PinId id) const noexcept {
    auto pin_pos = std::find_if(input_pins_.begin(), input_pins_.end(),
                                [id](const Pin &pin) { return pin.id_ == id; });

    if (pin_pos == input_pins_.end()) return nullptr;  // Not found

    return &(*pin_pos);
}

const Pin *Node::OutputPin(PinId id) const noexcept {
    auto pin_pos = std::find_if(output_pins_.begin(), output_pins_.end(),
                                [id](const Pin &pin) { return pin.id_ == id; });

    if (pin_pos == output_pins_.end()) {
        return nullptr;  // Not found
    } else {
        return &(*pin_pos);
    }
}

bool Node::HasProperty(PropertyId id) const noexcept {
    return properties_.find(id) != properties_.end();
}

Property *Node::GetProperty(PropertyId id) noexcept {
    auto property_pos = properties_.find(id);

    if (property_pos == properties_.end()) {
        return nullptr;  /// Not found
    } else {
        return &property_pos->second;
    }
}

const Property *Node::GetProperty(PropertyId id) const noexcept {
    auto property_pos = properties_.find(id);

    if (property_pos == properties_.end()) {
        return nullptr;  /// Not found
    } else {
        return &property_pos->second;
    }
}

PropertyId Node::AddProperty(Property property) {
    PropertyId id = property_id_count_++;
    properties_.insert_or_assign(id, property);

    return id;
}

void Node::SetProperty(PropertyId id, Property property) {
    properties_.insert_or_assign(id, property);  /// New methods my beloved
}

void Node::RemoveProperty(PropertyId id) { properties_.erase(id); }

#pragma endregion Public

#pragma region Private

Node::Node(NodeId id, NodeType type)
    : id_(id),
      type_(type),
      input_pin_count_(1),
      output_pin_count_(1),
      property_id_count_(1) {}

PinId Node::AddInputPin(Pin pin) {
    pin.id_ = input_pin_count_++;
    input_pins_.emplace_back(pin);
    return pin.id_;
}

PinId Node::AddOutputPin(Pin pin) {
    pin.id_ = output_pin_count_++;
    output_pins_.emplace_back(pin);
    return pin.id_;
}

void Node::RemoveInputPin(PinId pinid_) {
    auto pin_pos =
        std::find_if(input_pins_.begin(), input_pins_.end(),
                     [pinid_](const Pin &pin) { return pin.id_ == pinid_; });

    if (pin_pos == input_pins_.end()) {
        return;  /// Pin does not exists, nothing to do
    }

    input_pins_.erase(pin_pos);
}

void Node::RemoveOutputPin(PinId pinid_) {
    auto pin_pos =
        std::find_if(output_pins_.begin(), output_pins_.end(),
                     [pinid_](const Pin &pin) { return pin.id_ == pinid_; });

    if (pin_pos == output_pins_.end()) {
        return;  /// Pin does not exists, nothing to do
    }

    output_pins_.erase(pin_pos);
}

size_t Node::RemoveAllInputPins() {
    size_t count = input_pins_.size();
    input_pins_.clear();
    return count;
}

size_t Node::RemoveAllOutputPins() {
    size_t count = output_pins_.size();
    output_pins_.clear();
    return count;
}

size_t Node::RemoveAllPins() {
    return RemoveAllInputPins() + RemoveAllOutputPins();
}

#pragma endregion Private

}  // namespace core
