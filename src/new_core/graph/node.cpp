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

NodeId Node::id() const noexcept { return _id; }

NodeType Node::type() const noexcept { return _type; }

const std::vector<Pin> &Node::inputPins() const noexcept { return _input_pins; }

const std::vector<Pin> &Node::outputPins() const noexcept {
    return _output_pins;
}

size_t Node::inputPinsCount() const noexcept { return _input_pins.size(); }

size_t Node::outputPinsCount() const noexcept { return _output_pins.size(); }

bool Node::inputPinExists(PinId id) const noexcept {
    return std::find_if(_input_pins.begin(), _input_pins.end(),
                        [id](const Pin &pin) { return pin.id == id; }) !=
           _input_pins.end();
}

bool Node::outputPinExists(PinId id) const noexcept {
    return std::find_if(_output_pins.begin(), _output_pins.end(),
                        [id](const Pin &pin) { return pin.id == id; }) !=
           _output_pins.end();
}

const Pin *Node::inputPin(PinId id) const noexcept {
    auto pin_pos = std::find_if(_input_pins.begin(), _input_pins.end(),
                                [id](const Pin &pin) { return pin.id == id; });

    if (pin_pos == _input_pins.end()) return nullptr;  // Not found

    return &(*pin_pos);
}

const Pin *Node::outputPin(PinId id) const noexcept {
    auto pin_pos = std::find_if(_output_pins.begin(), _output_pins.end(),
                                [id](const Pin &pin) { return pin.id == id; });

    if (pin_pos == _output_pins.end())
        return nullptr;  // Not found
    else
        return &(*pin_pos);
}

bool Node::hasProperty(PropertyId id) const noexcept {
    return _properties.find(id) != _properties.end();
}

Property *Node::property(PropertyId id) noexcept {
    auto property_pos = _properties.find(id);

    if (property_pos == _properties.end())
        return nullptr;  /// Not found
    else
        return &property_pos->second;
}

const Property *Node::property(PropertyId id) const noexcept {
    auto property_pos = _properties.find(id);

    if (property_pos == _properties.end())
        return nullptr;  /// Not found
    else
        return &property_pos->second;
}

PropertyId Node::AddProperty(Property property) {
    PropertyId id = _property_id_count++;
    _properties.insert_or_assign(id, property);

    return id;
}

void Node::SetProperty(PropertyId id, Property property) {
    _properties.insert_or_assign(id, property);  /// New methods my beloved
}

void Node::RemoveProperty(PropertyId id) { _properties.erase(id); }

#pragma endregion Public

#pragma region Private

Node::Node(NodeId id, NodeType type)
    : _id(id),
      _type(type),
      _input_pin_count(1),
      _output_pin_count(1),
      _property_id_count(1) {}

PinId Node::AddInputPin(Pin pin) {
    pin.id = _input_pin_count++;
    _input_pins.emplace_back(pin);
    return pin.id;
}

PinId Node::AddOutputPin(Pin pin) {
    pin.id = _output_pin_count++;
    _output_pins.emplace_back(pin);
    return pin.id;
}

void Node::RemoveInputPin(PinId pin_id) {
    auto pin_pos =
        std::find_if(_input_pins.begin(), _input_pins.end(),
                     [pin_id](const Pin &pin) { return pin.id == pin_id; });

    if (pin_pos == _input_pins.end())
        return;  /// Pin does not exists, nothing to do

    _input_pins.erase(pin_pos);
}

void Node::RemoveOutputPin(PinId pin_id) {
    auto pin_pos =
        std::find_if(_output_pins.begin(), _output_pins.end(),
                     [pin_id](const Pin &pin) { return pin.id == pin_id; });

    if (pin_pos == _output_pins.end())
        return;  /// Pin does not exists, nothing to do

    _output_pins.erase(pin_pos);
}

size_t Node::RemoveAllInputPins() {
    size_t count = _input_pins.size();
    _input_pins.clear();
    return count;
}

size_t Node::RemoveAllOutputPins() {
    size_t count = _output_pins.size();
    _output_pins.clear();
    return count;
}

size_t Node::RemoveAllPins() {
    return RemoveAllInputPins() + RemoveAllOutputPins();
}

#pragma endregion Private

}  // namespace core
