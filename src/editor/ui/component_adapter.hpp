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

#include <cstddef>
#include <string>
#include <unordered_map>

#include "core/graph/datatypes.hpp"
#include "core/modules/capabilities/renderer_capability.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/** Draws renderer component descriptions and owns transient widget state. */
class ComponentAdapter final {
   public:
    void Draw(core::NodeId node_id, const core::capa::ComponentList &components,
              utils::WrappedRectangle bounds);

    void ClearNode(core::NodeId node_id) noexcept;
    void Clear() noexcept;
    bool HasState(core::NodeId node_id) const noexcept;

   private:
    struct NodeState {
        core::capa::ComponentId focused_component_ = 0;
        std::unordered_map<core::capa::ComponentId, std::string> text_;
    };

    std::unordered_map<core::NodeId, NodeState> states_;
};

}  // namespace editor::ui
