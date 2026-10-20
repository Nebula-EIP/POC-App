/**
 * @file renderer.hpp
 * @brief Components describing how the C nodes are edited
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#pragma once

#include <array>
#include <functional>
#include <string>
#include <string_view>

#include "graph/datatypes.hpp"
#include "modules/capabilities/renderer_capability.hpp"

namespace c_module::renderer {

namespace node_types {
inline constexpr core::NodeType kDeclaration = 1;
inline constexpr core::NodeType kLiteral = 2;
inline constexpr core::NodeType kBinaryExpression = 3;
inline constexpr core::NodeType kConditional = 4;
inline constexpr core::NodeType kLoop = 5;
inline constexpr core::NodeType kFunction = 6;
}  // namespace node_types

namespace properties {
inline constexpr core::PropertyId kName = 1;
inline constexpr core::PropertyId kValue = 2;
inline constexpr core::PropertyId kOperator = 3;
inline constexpr core::PropertyId kCondition = 4;
inline constexpr core::PropertyId kType = 5;
}  // namespace properties

/**
 * @brief Value types a variable or a literal can hold (C type names).
 *
 */
inline constexpr std::array<std::string_view, 5> kValueTypes = {
    "int", "float", "bool", "char", "string"};

/**
 * @brief Type used when a node has no (or an unknown) type property.
 *
 */
inline constexpr std::string_view kDefaultValueType = "int";

/**
 * @brief Operators offered by the binary expression node.
 *
 */
inline constexpr std::array<std::string_view, 5> kOperators = {"+", "-", "*",
                                                               "/", "%"};

/**
 * @brief Receives the properties changed by a component, applied together.
 *
 */
using PropertyUpdate =
    std::function<void(core::NodeId, const core::PropertyMap &)>;

/// Registers the C node descriptions without taking a dependency on Raylib.
void RegisterProviders(core::capa::RendererCapability &renderer,
                       PropertyUpdate update = {});

/**
 * @brief Value type stored in a node's type property.
 *
 * @return The type name, or kDefaultValueType when it is missing or unknown.
 */
std::string ValueType(const core::PropertyMap &properties);

/**
 * @brief Checks that a name can be used as a C/C++ identifier.
 *
 * @return An explanation in French, or an empty string when it is valid.
 */
std::string ValidateIdentifier(const std::string &name);

/**
 * @brief Checks that a text is a single printable character.
 *
 * @return An explanation in French, or an empty string when it is valid.
 */
std::string ValidateCharacter(const std::string &text);

/**
 * @brief Checks that a text can be written in a C string literal.
 *
 * @return An explanation in French, or an empty string when it is valid.
 */
std::string ValidateString(const std::string &text);

/**
 * @brief Converts a literal value to another value type.
 *
 * Used when the type of a literal changes, so the value keeps a meaning
 * (4.7 -> 5 as int, "true" as bool, 'a' as char, ...).
 *
 * @param value Current value (double, bool or std::string).
 * @param from Current value type.
 * @param to New value type.
 * @return The value stored for the new type.
 */
core::Property ConvertLiteralValue(const core::Property &value,
                                   std::string_view from, std::string_view to);

}  // namespace c_module::renderer
