/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 26-09-2026
 */

 #pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "graph/graph.hpp"
#include "node_view.hpp"

namespace editor::ui {

class NodeCanvas final {
   public:
    void AddNode(const core::Node &node, std::string title,
                 utils::WrappedVector2 position);
    void RemoveMissingNodes(const core::Graph &graph);
    void ProcessInput();
    void Draw() const;

    Camera &GetCamera() noexcept { return camera_; }
    const std::unordered_map<core::NodeId, std::unique_ptr<NodeView>> &Views()
        const noexcept {
        return views_;
    }

   private:
    void SelectOnly(core::NodeId node_id);
    void SelectInRectangle(utils::WrappedRectangle rectangle);
    std::optional<HitResult> HitTest(utils::WrappedVector2 point) const;

    Camera camera_;
    std::unordered_map<core::NodeId, std::unique_ptr<NodeView>> views_;
    std::vector<core::NodeId> draw_order_;
    std::optional<utils::WrappedVector2> selection_start_;
    std::optional<utils::WrappedVector2> drag_start_;
    std::unordered_map<core::NodeId, utils::WrappedVector2> drag_origins_;
};

}  // namespace editor::ui
