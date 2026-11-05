/**
 * @file node_view.hpp
 * @brief Declaration of the NodeView class, which represents a visual node in
 * the editor's UI.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 05-11-2026
 */

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "graph/node.hpp"
#include "render/camera.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief Represents the different parts of a node that can be hit by a mouse
 * click.
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

/**
 * @brief What the nodes need from the rest of the editor to be drawn in the
 * project's colors (see utils/palette.hpp).
 *
 * Every member is optional: without it, pins are drawn in a neutral color,
 * filled, and the header has no accent.
 */
struct NodePaint {
    /// Name of a pin type ("int", "void"...): picks its color and shape.
    std::function<std::string(core::DataType)> type_name_;
    /// Color of the line at the top of a node type (palette::NodeAccent()),
    /// fully transparent for none.
    std::function<utils::WrappedColor(core::NodeType)> accent_;
    /// Whether a pin has a link: filled when linked, hollow otherwise.
    std::function<bool(core::NodeId, core::PinId, bool is_input)> pin_linked_;
};

/**
 * @brief Represents a view of a node in the editor's UI.
 */
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
     * @brief Marks a pin as having an error (e.g., failed connection).
     */
    void SetErrorPin(core::PinId pin, bool is_input) noexcept;

    /**
     * @brief Clears the error state of pins.
     */
    void ClearErrorPin() noexcept { error_pin_id_ = 0; }

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
     * @brief Gets the world position of a specific pin.
     *
     * @param pin_id The ID of the pin.
     *
     * @return The position of the pin, or std::nullopt if the pin is not found.
     */
    std::optional<utils::WrappedVector2> GetPinPosition(
        core::PinId pin_id, bool is_input) const noexcept;

    /**
     * @brief Rebuilds the pins after they changed in the graph (type, name).
     *
     * The position of the node is kept.
     *
     * @param node The node this view displays.
     */
    void Refresh(const core::Node &node);

    /**
     * @brief Reserves room under the pins for the node's fields.
     *
     * The node grows to fit; {0, 0} removes the room.
     *
     * @param size Width and height the fields need.
     */
    void SetContentSize(utils::WrappedVector2 size);

    /**
     * @brief Area where the node's fields are drawn (world space).
     *
     * Empty (zero height) when no content size was set.
     */
    [[nodiscard]] utils::WrappedRectangle ContentBounds() const noexcept;

    /**
     * @brief Draws the node: shadow, card, header accent, title and pins.
     *
     * @param camera The camera to use for drawing.
     * @param paint Type names, categories and links, for the colors.
     */
    void Draw(const render::Camera &camera, const NodePaint &paint = {}) const;

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
     * @brief Draws one pin on the edge of the node, and its name.
     *
     * @param port The pin.
     * @param is_input Whether it is on the left (input) or right edge.
     * @param paint Type names and links, for the color and shape.
     */
    void DrawPort(const PortView &port, bool is_input,
                  const NodePaint &paint) const;

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
    void Layout();

    core::NodeId node_id_;
    core::NodeType node_type_;
    std::string title_;
    utils::WrappedRectangle bounds_{};
    std::vector<PortView> inputs_;
    std::vector<PortView> outputs_;
    utils::WrappedVector2 content_size_{0.0F, 0.0F};
    bool selected_ = false;
    core::PinId error_pin_id_ = 0;
    bool error_pin_is_input_ = false;
};

}  // namespace editor::ui