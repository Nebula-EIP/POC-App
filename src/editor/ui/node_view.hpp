/**
 * @file node_view.hpp
 * @brief Declaration of the NodeView class, which represents a visual node in the editor's UI.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 26-09-2026
 */

#pragma once

#include <string>
#include <vector>

#include "graph/node.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

struct Camera {
    utils::WrappedVector2 offset_{0.0F, 0.0F};
    float zoom_ = 1.0F;

    /**
     *@brief Converts a screen position to a world position.
     *
     * @param point The screen position.
     *
     * @return The corresponding world position.
     */
    utils::WrappedVector2 ScreenToWorld(
        utils::WrappedVector2 point) const noexcept;

    /**
     * @brief Converts a world position to a screen position.
     *
     * @param point The world position.
     *
     * @return The corresponding screen position.
     */
    utils::WrappedVector2 WorldToScreen(
        utils::WrappedVector2 point) const noexcept;
};

 /**
  * @brief Represents the different parts of a node that can be hit by a mouse click.
 */
enum class HitPart { kNone, kBody, kInputPin, kOutputPin };

/**
 * @brief Represents the result of a hit test on a node.
 *
 * @param node_id The ID of the node that was hit, or 0 if no node was hit.
 * @param pin_id The ID of the pin that was hit, or 0 if no pin was hit.
 * @param part The part of the node that was hit.
 */
struct HitResult {
    core::NodeId node_id_ = 0;
    core::PinId pin_id_ = 0;
    HitPart part_ = HitPart::kNone;
};

class NodeView final {
   public:
   /**
    * @brief Constructs a new NodeView.
    *
    * @param node The core node to view.
    * @param title The title of the node.
    * @param position The position of the node.
    */
    NodeView(const core::Node &node, std::string title,
             utils::WrappedVector2 position);

    /**
     * @brief Gets the ID of the node.
     *
     * @return The ID of the node.
     */
    core::NodeId NodeId() const noexcept { return node_id_; }

    /**
     * @brief Gets the bounds of the node.
     *
     * @return The bounds of the node.
     */
    const utils::WrappedRectangle &Bounds() const noexcept { return bounds_; }

    /**
     * @brief Gets the title of the node.
     *
     * @return The title of the node.
     */
    const std::string &Title() const noexcept { return title_; }

    /**
     * @brief Gets the selected state of the node.
     *
     * @return True if the node is selected, false otherwise.
     */
    bool Selected() const noexcept { return selected_; }

    /**
     * @brief Sets the selected state of the node.
     *
     * @param selected The new selected state.
     */
    void SetSelected(bool selected) noexcept { selected_ = selected; }

    /**
     * @brief Moves the node by the specified delta.
     *
     * @param delta The amount to move the node.
     */
    void MoveBy(utils::WrappedVector2 delta) noexcept;

    /**
     * @brief Performs a hit test on the node.
     *
     * @param world_point The point in world coordinates to test.
     *
     * @return The result of the hit test.
     */
    HitResult HitTest(utils::WrappedVector2 world_point) const noexcept;

    /**
     * @brief Draws the node.
     *
     * @param camera The camera to use for drawing.
     */
    void Draw(const Camera &camera) const;

   private:
   /**
    * @brief Represents a view of a port on the node.
    *
    * @param id The ID of the port.
    * @param name The name of the port.
    * @param type The data type of the port.
    * @param bounds The bounds of the port in the node's coordinate system.
    */
    struct PortView {
        core::PinId id_;
        std::string name_;
        core::DataType type_;
        utils::WrappedRectangle bounds_;
    };

    /**
     * @brief Gets the color associated with a data type.
     *
     * @param type The data type.
     *
     * @return The color associated with the data type.
     */
    static utils::WrappedColor TypeColor(core::DataType type) noexcept;

    /**
     * @brief Gets the width of a string when rendered.
     *
     * @param text The string to measure.
     *
     * @return The width of the string.
     */
    static float TextWidth(const std::string &text) noexcept;

    /**
     * @brief Rebuilds the layout of the node based on its current state.
     *
     * @param node The core node to use for layout.
     */
    void RebuildLayout(const core::Node &node);

    core::NodeId node_id_;
    core::NodeType node_type_;
    std::string title_;
    utils::WrappedRectangle bounds_{};
    std::vector<PortView> inputs_;
    std::vector<PortView> outputs_;
    bool selected_ = false;
};

}  // namespace editor::ui
