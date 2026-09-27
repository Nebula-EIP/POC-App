/**
 * @file node_canvas.hpp
 * @brief Declaration of the NodeCanvas class, which manages the display and interaction of nodes in the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 26-09-2026
 */

#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "graph/graph.hpp"
#include "node_view.hpp"

namespace editor::ui {

class NodeCanvas final {
   public:
   /**
    * @brief Adds a new node to the canvas.
    *
    * @param node The node to add.
    * @param title The title of the node.
    * @param position The position of the node.
    */
    void AddNode(const core::Node &node, std::string title,
                 utils::WrappedVector2 position);

    /**
     * @brief Removes nodes from the canvas that are not present in the graph.
     *
     * @param graph The graph to compare against.
     */
    void RemoveMissingNodes(const core::Graph &graph);

    /**
     * @brief Processes user input for the canvas.
     */
    void ProcessInput();

    /**
     * @brief Draws the canvas and all its nodes.
     */
    void Draw() const;

    /**
     * @brief Gets the camera used by the canvas.
     *
     * @return A reference to the camera.
     */
    Camera &GetCamera() noexcept { return camera_; }

    /**
     * @brief Gets the views of the nodes in the canvas.
     *
     * @return A reference to the map of node views.
     */
    const std::unordered_map<core::NodeId, std::unique_ptr<NodeView>> &Views()
        const noexcept {
        return views_;
    }

   private:
   /**
    * @brief Selects only the specified node.
    *
    * @param node_id The ID of the node to select.
    */
    void SelectOnly(core::NodeId node_id);

    /**
     * @brief Selects all nodes within a rectangle.
     *
     * @param rectangle The rectangle to select nodes in.
     */
    void SelectInRectangle(utils::WrappedRectangle rectangle);

    /**
     * @brief Performs a hit test on the canvas.
     *
     * @param point The point to test.
     *
     * @return The result of the hit test, or std::nullopt if no hit occurred.
     */
    std::optional<HitResult> HitTest(utils::WrappedVector2 point) const;

    Camera camera_;
    std::unordered_map<core::NodeId, std::unique_ptr<NodeView>> views_;
    std::vector<core::NodeId> draw_order_;
    std::optional<utils::WrappedVector2> selection_start_;
    std::optional<utils::WrappedVector2> drag_start_;
    std::unordered_map<core::NodeId, utils::WrappedVector2> drag_origins_;
};

}  // namespace editor::ui
