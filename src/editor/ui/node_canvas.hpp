/**
 * @file node_canvas.hpp
 * @brief Declaration of the NodeCanvas class, which manages the display and
 * interaction of nodes in the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 24-10-2026
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "graph/graph.hpp"
#include "node_view.hpp"
#include "render/camera.hpp"

namespace editor::ui {

/**
 * @brief Mouse and keyboard state the canvas reacts to, for one frame.
 *
 * Kept separate from raylib so that the canvas can be tested without a
 * window. Use NodeCanvas::ReadInput() in the application.
 */
struct CanvasInput {
    utils::WrappedVector2 cursor_{0.0F, 0.0F};  ///< In world space.
    bool left_pressed_ = false;
    bool left_down_ = false;
    bool additive_ = false;  ///< Ctrl held: the selection is extended.
    /// The cursor is inside the window. A link released outside it is
    /// dropped without opening anything.
    bool in_window_ = true;
    /// Another part of the editor (top bar, popup) owns the pointer this
    /// frame: presses are ignored and a released drag ends without creating
    /// anything.
    bool pointer_captured_ = false;
};

/**
 * @brief A link drag released on empty canvas: no pin, no node.
 */
struct LinkDrop {
    HitResult pin_;                ///< The pin the drag started from.
    utils::WrappedVector2 world_;  ///< Where it was released, in world space.
};

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
     * Any drag in progress on a removed node (move or connection) is
     * cancelled.
     *
     * @param graph The graph to compare against.
     */
    void RemoveMissingNodes(const core::Graph &graph);

    /**
     * @brief Gets the selected nodes, from back to front (draw order).
     *
     * @return The ids of the selected nodes.
     */
    [[nodiscard]] std::vector<core::NodeId> SelectedNodes() const;

    /**
     * @brief Selects exactly the given nodes (unknown ids are ignored) and
     * brings them to the front, keeping their relative order.
     *
     * @param node_ids The nodes to select.
     */
    void SetSelection(const std::vector<core::NodeId> &node_ids);

    /**
     * @brief Performs a hit test on the canvas, front-most node first.
     *
     * @param point The point to test, in world space.
     *
     * @return The result of the hit test, or std::nullopt if no hit occurred.
     */
    [[nodiscard]] std::optional<HitResult> HitTest(
        utils::WrappedVector2 point) const;

    /**
     * @brief Reads this frame's input from raylib.
     *
     * @param camera The camera used to convert cursor position to world
     * space. Its viewport is the window size.
     * @param pointer_captured Whether another part of the editor owns the
     * pointer this frame.
     */
    [[nodiscard]] static CanvasInput ReadInput(const render::Camera &camera,
                                               bool pointer_captured = false);

    /**
     * @brief Processes user input for the canvas, read from raylib.
     *
     * @param camera The camera used to convert cursor position to world
     * space.
     * @param pointer_captured Whether another part of the editor owns the
     * pointer this frame.
     */
    void ProcessInput(const render::Camera &camera,
                      bool pointer_captured = false);

    /**
     * @brief Handles one frame of input: selection, node moves, and link
     * drags. A link drag released on a pin requests a connection
     * (PopPendingConnectionRequest); released on empty canvas inside the
     * window, it records a link drop (PopPendingLinkDrop). Released on a node
     * body, outside the window, or while the pointer is captured, it ends
     * without anything.
     */
    void Update(const CanvasInput &input);

    /**
     * @brief Draws what a node holds (its fields), right after the node itself.
     *
     */
    using NodeContentDrawer = std::function<void(const NodeView &view)>;

    /**
     * @brief Draws the canvas and all its nodes.
     *
     * @param camera The camera the nodes are drawn against.
     * @param draw_content Optional, called after each node so its fields are
     * covered by the nodes drawn above it.
     */
    void Draw(const render::Camera &camera,
              const NodeContentDrawer &draw_content = {}) const;

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
     * @brief Gets the selection rectangle being dragged, in world space.
     *
     * @return The normalized rectangle (positive width and height), or
     * std::nullopt when no rectangle selection is in progress.
     */
    [[nodiscard]] std::optional<utils::WrappedRectangle>
    ActiveSelectionRectangle() const noexcept;

    /**
     * @brief Polls if a connection drag just ended and returns the requested
     * connection.
     */
    std::optional<std::pair<HitResult, HitResult>>
    PopPendingConnectionRequest() noexcept;

    /**
     * @brief Polls if a link drag just ended on empty canvas.
     *
     * @return The pin the drag started from and the drop position, once.
     */
    std::optional<LinkDrop> PopPendingLinkDrop() noexcept;

    /**
     * @brief Gets the current drag state for drawing ghost links.
     */
    std::optional<std::pair<utils::WrappedVector2, utils::WrappedVector2>>
    GetConnectionDragLine() const noexcept;

   private:
    /**
     * @brief Selects only the specified node.
     *
     * @param node_id The ID of the node to select.
     */
    void SelectOnly(core::NodeId node_id);

    /**
     * @brief Selects the nodes touching a rectangle, plus the nodes of
     * selection_base_ (kept when the drag started with Ctrl held).
     *
     * @param rectangle The rectangle to select nodes in.
     */
    void SelectInRectangle(utils::WrappedRectangle rectangle);

    /**
     * @brief Draws the selection rectangle being dragged, if any.
     *
     * @param camera The camera, to keep the border 1 pixel wide on screen.
     */
    void DrawSelectionRectangle(const render::Camera &camera) const;

    std::unordered_map<core::NodeId, std::unique_ptr<NodeView>> views_;
    std::vector<core::NodeId> draw_order_;
    std::optional<utils::WrappedVector2> selection_start_;
    std::optional<utils::WrappedVector2> selection_current_;
    std::vector<core::NodeId> selection_base_;
    std::optional<utils::WrappedVector2> drag_start_;
    std::unordered_map<core::NodeId, utils::WrappedVector2> drag_origins_;

    std::optional<HitResult> connection_drag_start_;
    std::optional<utils::WrappedVector2> connection_drag_current_;
    std::optional<std::pair<HitResult, HitResult>> pending_connection_;
    std::optional<LinkDrop> pending_link_drop_;
};

}  // namespace editor::ui