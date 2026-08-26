/**
 * @file datatypes.cpp
 * @brief Declares all types commonly used in the core library
 *
 * @author Created by JeanBizeul
 * @date Created on 01-08-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 26-08-2026
 */

#pragma once

#include <any>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace core {
using NodeId = uint32_t;          ///< Unique id for each Node
using PinId = uint16_t;           ///< Unique id for each Node's Pin
using DataType = uint16_t;        ///< Unique id for each type declared by modules
using NodeType = uint16_t;        ///< Unique id for each node declared by modules
using ModuleId = uint8_t;         ///< Unique id for each loaded module
using PropertyId = uint16_t;      ///< Unique id for each Node's properties
using PropertyTypeId = uint32_t;  ///< Unique id for each property type

/**
 * @brief Represents a connection between two nodes
 */
struct Connection {
    NodeId from_node;
    PinId from_pin;  // aka output pin

    NodeId to_node;
    PinId to_pin;  // aka input pin

    DataType data_type;
};

/**
 * @brief Represents a connection point of a node
 */
struct Pin {
    PinId id;
    std::string name;
    DataType type;
};

/**
 * @brief Represents a type
 */
struct Property {
    PropertyTypeId type_id;
    std::any value;     ///< Free storage space for the property's values
};

using PropertyMap = std::unordered_map<PropertyId, Property>;
}  // namespace core
