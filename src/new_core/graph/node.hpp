/**
 * @file datatypes.hpp
 * @brief Declares all types commonly used in the core library
 *
 * @author Created by JeanBizeul
 * @date Created on 01-08-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 27-08-2026
 */

#pragma once

#include <cstdint>
#include <vector>

#include "datatypes.hpp"

namespace core {

class Graph; ///< Forward def to make it a friend

/**
 * @brief Data storage representing a Node in the Graph.
 */
class Node {
   public:
    ~Node() = default;

    Node(Node &&) = default;
    Node &operator=(Node &&) = default;

    /**
     * @brief Retrieves the node's unique id
     *
     * @return The node's unique id
     */
    NodeId id() const noexcept;

    /**
     * @brief Retrieves the node's type
     *
     * @return The node's type
     */
    NodeType type() const noexcept;

    /**
     * @brief Retrieves the node's input pins
     *
     * @return A constant reference to the vector of input pins
     */
    const std::vector<Pin> &inputPins() const noexcept;

    /**
     * @brief Retrieves the node's output pins
     *
     * @return A constant reference to the vector of output pins
     */
    const std::vector<Pin> &outputPins() const noexcept;

    /**
     * @brief Retrieves the number of input pins
     *
     * @return The number of input pins
     */
    size_t inputPinsCount() const noexcept;

    /**
     * @brief Retrieves the number of output pins
     *
     * @return The number of output pins
     */
    size_t outputPinsCount() const noexcept;

    /**
     * @brief Checks if a node has any input pins
     *
     * @param pin The pin id to check for existence
     *
     * @return true if the node has input pins, false otherwise
     */
    bool inputPinExists(PinId pin) const noexcept;

    /**
     * @brief Checks if a node has any output pins
     *
     * @param pin The pin id to check for existence
     *
     * @return true if the node has output pins, false otherwise
     */
    bool outputPinExists(PinId pin) const noexcept;

    /**
     * @brief Retrieves a pointer to the input pin with the specified id
     *
     * @param pin_id The id of the pin to retrieve
     *
     * @return A pointer to the input pin if it exists, nullptr otherwise
     */
    const Pin *inputPin(PinId pin_id) const noexcept;

    /**
     * @brief Retrieves a pointer to the output pin with the specified id
     *
     * @param pin_id The id of the pin to retrieve
     *
     * @return A pointer to the output pin if it exists, nullptr otherwise
     */
    const Pin *outputPin(PinId pin_id) const noexcept;

    /**
     * @brief Checks if the node has a property with the specified id
     *
     * @param id The id of the property to check for existence
     *
     * @return true if the node has the property, false otherwise
     */
    bool hasProperty(PropertyId id) const noexcept;

    /**
     * @brief Retrieves a pointer to the property with the specified id
     *
     * @param id The id of the property to retrieve
     *
     * @return A pointer to the property if it exists, nullptr otherwise
     */
    Property *property(PropertyId) noexcept;

    /**
     * @brief Retrieves a const pointer to the property with the specified id
     *
     * @param id The id of the property to retrieve
     *
     * @return A const pointer to the property if it exists, nullptr otherwise
     */
    const Property *property(PropertyId) const noexcept;

    /**
     * @brief Sets a property with the specified id and value
     * Properties are data storage reserved for module usage.
     * They can store any kind of data specified by the type_list capability.
     *
     * @param id The id of the property to set
     * @param property The property to set
     */
    void SetProperty(PropertyId, Property);

    /**
     * @brief Removes a property with the specified id
     * Properties are data storage reserved for module usage.
     * They can store any kind of data specified by the type_list capability.
     *
     * @param id The id of the property to remove
     */
    void RemoveProperty(PropertyId);

   private:
    friend core::Graph; ///< This is required to allow the Graph operations to be more effective

    Node() = default;

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    /**
     * @brief Adds an input pin to the node
     *
     * @param pin The pin to add
     */
    void AddInputPin(Pin pin);
    /**
     * @brief Adds an output pin to the node
     *
     * @param pin The pin to add
     */
    void AddOutputPin(Pin pin);

    /**
     * @brief Removes an input pin from the node
     *
     * @param pin_id The id of the pin to remove
     */
    void RemoveInputPin(PinId pin_id);
    /**
     * @brief Removes an output pin from the node
     *
     * @param pin_id The id of the pin to remove
     */
    void RemoveOutputPin(PinId pin_id);

    /**
     * @brief Removes all input pins from the node
     *
     * @return The number of pins removed
     */
    size_t RemoveAllInputPins() noexcept;
    /**
     * @brief Removes all output pins from the node
     *
     * @return The number of pins removed
     */
    size_t RemoveAllOutputPins() noexcept;
    /**
     * @brief Removes all pins from the node
     *
     * @return The number of pins removed
     */
    size_t RemoveAllPins() noexcept;

    NodeId _id;      ///< unique node id
    NodeType _type;  ///< Node type from the module

    std::vector<Pin> _input_pins;
    std::vector<Pin> _output_pins;

    PropertyMap _properties; ///< properties are data storages reserved for module usage
};

}  // namespace core
