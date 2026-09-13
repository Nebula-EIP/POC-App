/**
 * @file datatypes.hpp
 * @brief Declares all types commonly used in the core library
 *
 * @author Created by JeanBizeul
 * @date Created on 01-08-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 10-09-2026
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
using NodeId = uint32_t;        ///< Unique id for each Node
using PinId = uint16_t;         ///< Unique id for each Node's Pin
using ConnectionId = uint32_t;  ///< Unique id for each Graph's Connection
using DataType = uint16_t;      ///< Unique id for each type declared by modules
using NodeType = uint16_t;      ///< Unique id for each node declared by modules
using ModuleId = uint8_t;       ///< Unique id for each loaded module
using PropertyId = uint16_t;    ///< Unique id for each Node's properties
using PropertyTypeId = uint32_t;  ///< Unique id for each property type

/**
 * @brief Represents a connection between two nodes
 */
struct Connection {
    NodeId from_node_;
    PinId out_pin_;

    NodeId to_node_;
    PinId in_pin_;

    DataType data_type_;
};

/**
 * @brief Represents a connection point of a node
 */
struct Pin {
    PinId id_;
    std::string name_;
    DataType type_;
};

/**
 * @brief Represents a typed value storage
 */
struct Property {
    PropertyTypeId type_id_;
    std::any value_;  ///< Free storage space for the property's values
};

using PropertyMap = std::unordered_map<PropertyId, Property>;
}  // namespace core
