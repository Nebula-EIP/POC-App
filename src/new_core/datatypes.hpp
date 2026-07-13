/**
 * This files contains all datatypes widely used in the core lib
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
using NodeId = uint32_t;          // Unique id for each Node
using PinId = uint8_t;            // Unique id for each Node's Pin
using DataType = uint16_t;        // Unique id for each type declared by modules
using NodeType = uint16_t;        // Unique id for each node declared by modules
using ModuleId = uint8_t;         // Unique id for each loaded module
using PropertyId = uint16_t;      // Unique id for each Node's properties
using PropertyTypeId = uint32_t;  // Unique id for each property type

struct Connection {
    NodeId from_node;
    PinId from_pin;  // aka output pin

    NodeId to_node;
    PinId to_pin;  // aka input pin

    DataType data_type;
};

struct Pin {
    PinId id;
    std::string name;
    DataType type;
};

struct Property {
    PropertyTypeId type_id;
    std::any value;
};

using PropertyMap = std::unordered_map<PropertyId, Property>;
}  // namespace core
