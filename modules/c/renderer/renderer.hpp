/**
 * @file create_module.cpp
 * @brief Hook used to retreive the C module
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 20-09-2026
 */

#pragma once

#include <functional>

#include "graph/datatypes.hpp"
#include "modules/capabilities/renderer_capability.hpp"

namespace c_module::renderer {

namespace node_types {
inline constexpr core::NodeType kDeclaration = 1;
inline constexpr core::NodeType kLiteral = 2;
inline constexpr core::NodeType kBinaryExpression = 3;
inline constexpr core::NodeType kConditional = 4;
inline constexpr core::NodeType kLoop = 5;
}  // namespace node_types

namespace properties {
inline constexpr core::PropertyId kName = 1;
inline constexpr core::PropertyId kValue = 2;
inline constexpr core::PropertyId kOperator = 3;
inline constexpr core::PropertyId kCondition = 4;
inline constexpr core::PropertyId kType = 5;
}  // namespace properties

using PropertyUpdate =
    std::function<void(core::NodeId, core::PropertyId, core::Property)>;

/// Registers the C node descriptions without taking a dependency on Raylib.
void RegisterProviders(core::capa::RendererCapability &renderer,
                       PropertyUpdate update = {});

}  // namespace c_module::renderer
