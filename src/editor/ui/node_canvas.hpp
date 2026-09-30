/**
 * @file node_canvas.hpp
 * @brief Declaration of the NodeCanvas class, which manages the display and
 * interaction of nodes in the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 27-09-2026
 */

#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "graph/graph.hpp"
#include "node_view.hpp"
#include "render/camera.hpp"

namespace editor::ui {

/**
 * @brief Represents a canvas for displaying and interacting with nodes.
 *
 * Takes the render::Camera it draws and hit-tests against from the caller
 * rather than owning one, so there is a single camera (Application's) for
 * the whole editor — never two that could drift out of sync on pan/zoom.
 */
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
     *
     * @param camera The camera used to convert cursor position to world
     * space.
     */
    void ProcessInput(const render::Camera &camera);

    /**
     * @brief Draws the canvas and all its nodes.
     *
     * @param camera The camera the nodes are drawn against.
     */
    void Draw(const render::Camera &camera) const;

    /**
     * @brief Gets the views of the nodes in the canvas.
     *
     * @return A reference to the map of node views.
     */
    const std::unordered_map<core::NodeId, std::unique_ptr<NodeView>> &Views()
        const noexcept {
        return views_;
    }

    /**
     * @brief Polls if a connection drag just ended and returns the requested
     * connection.
     */
    std::optional<std::pair<HitResult, HitResult>>
    PopPendingConnectionRequest() noexcept {
        auto result = pending_connection_;
        pending_connection_ = std::nullopt;
        return result;
    }

    /**
     * @brief Gets the current drag state for drawing ghost links.
     */
    std::optional<std::pair<utils::WrappedVector2, utils::WrappedVector2>>
    GetConnectionDragLine() const noexcept {
        if (connection_drag_start_ && connection_drag_current_) {
            if (auto start_view = views_.find(connection_drag_start_->node_id_);
                start_view != views_.end()) {
                if (auto start_pos = start_view->second->GetPinPosition(
                        connection_drag_start_->pin_id_,
                        connection_drag_start_->part_ == HitPart::kInputPin)) {
                    return std::make_pair(*start_pos,
                                          *connection_drag_current_);
                }
            }
        }
        return std::nullopt;
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
     * @param point The point to test, in world space.
     *
     * @return The result of the hit test, or std::nullopt if no hit occurred.
     */
    std::optional<HitResult> HitTest(utils::WrappedVector2 point) const;

    std::unordered_map<core::NodeId, std::unique_ptr<NodeView>> views_;
    std::vector<core::NodeId> draw_order_;
    std::optional<utils::WrappedVector2> selection_start_;
    std::optional<utils::WrappedVector2> drag_start_;
    std::unordered_map<core::NodeId, utils::WrappedVector2> drag_origins_;

    std::optional<HitResult> connection_drag_start_;
    std::optional<utils::WrappedVector2> connection_drag_current_;
    std::optional<std::pair<HitResult, HitResult>> pending_connection_;
};

}  // namespace editor::ui