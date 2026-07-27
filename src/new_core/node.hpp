#pragma once

#include <cstdint>
#include <vector>

#include "datatypes.hpp"

namespace core {

// Forward def to make it a friendé"
class Graph;

/**
 * @brief Data storage representing a Node in the Graph.
 */
class Node {
   public:
    ~Node() = default;

    Node(Node &&) = default;
    Node &operator=(Node &&) = default;

    NodeId id() const noexcept;
    NodeType type() const noexcept;

    const std::vector<Pin> &inputPins() const noexcept;
    const std::vector<Pin> &outputPins() const noexcept;

    size_t inputPinsCount() const noexcept;
    size_t outputPinsCount() const noexcept;

    bool inputPinExists(PinId pin) const noexcept;
    bool outputPinExists(PinId pin) const noexcept;

    const Pin *inputPin(PinId pin_id) const noexcept;
    const Pin *outputPin(PinId pin_id) const noexcept;

    bool hasProperty(PropertyId id) const noexcept;

    const Property *property(PropertyId) const noexcept;
    Property *property(PropertyId) noexcept;

    void SetProperty(PropertyId, Property);

    void RemoveProperty(PropertyId);

   private:
    // This is required to allow the Graph operations to be more effective
    friend core::Graph;

    Node() = default;

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    void AddInputPin(Pin pin);
    void AddOutputPin(Pin pin);

    void RemoveInputPin(PinId pin_id);
    void RemoveOutputPin(PinId pin_id);

    size_t RemoveAllInputPins() noexcept;
    size_t RemoveAllOutputPins() noexcept;
    size_t RemoveAllPins() noexcept;

    NodeId _id;      // unique node id
    NodeType _type;  // Node type from the module

    std::vector<Pin> _input_pins;
    std::vector<Pin> _output_pins;

    // properties are data storages reserved for module usage
    PropertyMap _properties;
};

}  // namespace core
