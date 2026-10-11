/**
 * @file component_adapter.hpp
 * @brief Draws the components a module describes for a node and turns user
 * input on them into the components' callbacks.
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/graph/datatypes.hpp"
#include "core/modules/capabilities/renderer_capability.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief Input of one frame for the component fields.
 *
 * The cursor is in the same space as the bounds given to the adapter (world
 * space when the fields are drawn inside the nodes).
 */
struct ComponentInput {
    utils::WrappedVector2 cursor_{0.0F, 0.0F};
    bool left_pressed_ = false;
    bool left_down_ = false;
    std::string typed_;  ///< UTF-8 text typed this frame
    bool backspace_ = false;
    bool enter_ = false;
    bool escape_ = false;

    /// Reads this frame's mouse and keyboard; drains the typed characters.
    static ComponentInput Read(utils::WrappedVector2 cursor);
};

/**
 * @brief Draws renderer component descriptions and owns transient widget
 * state (focused field, text being typed, open list).
 *
 * Components are rebuilt from the graph every frame: the adapter never keeps
 * a value of its own, except the text being typed. Text and number fields are
 * applied on Enter or when clicking elsewhere, and only when valid; Escape
 * cancels. While a text is invalid its explanation is drawn under the field.
 */
class ComponentAdapter final {
   public:
    /// Height of one component row.
    static constexpr float kRowHeight = 24.0F;

    /**
     * @brief Space the components need to be drawn.
     *
     * @return Width and height; {0, 0} for an empty list.
     */
    static utils::WrappedVector2 PreferredSize(
        const core::capa::ComponentList &components);

    /**
     * @brief Rectangle of the interactive part of a component.
     *
     * @param components All the components of the node.
     * @param bounds Area given to the components.
     * @param index Index of the component in the list.
     */
    static utils::WrappedRectangle ControlBounds(
        const core::capa::ComponentList &components,
        utils::WrappedRectangle bounds, std::size_t index);

    /**
     * @brief Rectangle of one option of an open select list.
     */
    static utils::WrappedRectangle OptionBounds(
        const core::capa::ComponentList &components,
        utils::WrappedRectangle bounds, std::size_t index, std::size_t option);

    /**
     * @brief Handles one frame of input for the node being edited.
     *
     * @param node_id The edited node (the selected one).
     * @param components Its components.
     * @param bounds Area where they are drawn.
     * @param input This frame's input.
     *
     * @return true when the click was on one of the fields (or an open
     * list), so nothing else should use it.
     */
    bool Update(core::NodeId node_id,
                const core::capa::ComponentList &components,
                utils::WrappedRectangle bounds, const ComponentInput &input);

    /**
     * @brief Draws the components.
     *
     * @param active true when the node can be edited (it is selected).
     */
    void Draw(core::NodeId node_id, const core::capa::ComponentList &components,
              utils::WrappedRectangle bounds, bool active) const;

    /// True while a text or number field has the keyboard.
    [[nodiscard]] bool IsEditingText() const noexcept;

    /// True while a select list is open.
    [[nodiscard]] bool IsListOpen() const noexcept;

    /// Node of the focused field or open list, if any.
    [[nodiscard]] std::optional<core::NodeId> FocusedNode() const noexcept;

    /// Text being typed in the focused field (empty when none).
    [[nodiscard]] const std::string &EditedText() const noexcept;

    /// Explanation of why the typed text is invalid (empty when valid).
    [[nodiscard]] const std::string &EditError() const noexcept;

    /// Drops the current edit and closes any list, without applying.
    void CancelEdit() noexcept;

    /**
     * @brief Takes the messages of edits that were dropped because invalid.
     *
     * @return The messages, oldest first; the adapter forgets them.
     */
    std::vector<std::string> TakeMessages();

    void ClearNode(core::NodeId node_id) noexcept;
    void Clear() noexcept;
    bool HasState(core::NodeId node_id) const noexcept;

    /**
     * @brief Checks a number typed by the user.
     *
     * Accepts a comma as decimal separator.
     *
     * @param text The typed text.
     * @param field The number field (integer flag and bounds).
     * @param value Receives the parsed value when valid.
     * @return An explanation in French, empty when valid.
     */
    static std::string ValidateNumber(const std::string &text,
                                      const core::capa::NumberField &field,
                                      double *value);

    /// Text shown for a number (no decimals for an integer field).
    static std::string FormatNumber(double value, bool integer);

   private:
    struct Edit {
        core::NodeId node_id_ = 0;
        core::capa::ComponentId component_id_ = 0;
        std::string text_;
        std::string error_;
    };

    struct OpenList {
        core::NodeId node_id_ = 0;
        core::capa::ComponentId component_id_ = 0;
    };

    void StartEdit(core::NodeId node_id,
                   const core::capa::Component &component);
    void UpdateEditError(const core::capa::Component &component);
    void FinishEdit(const core::capa::ComponentList &components, bool apply);
    bool HandleOpenList(const core::capa::ComponentList &components,
                        utils::WrappedRectangle bounds,
                        const ComponentInput &input);
    void DrawRow(const core::capa::ComponentList &components,
                 utils::WrappedRectangle bounds, std::size_t index,
                 core::NodeId node_id, bool active) const;
    void DrawOverlay(core::NodeId node_id,
                     const core::capa::ComponentList &components,
                     utils::WrappedRectangle bounds) const;

    std::optional<Edit> edit_;
    std::optional<OpenList> open_list_;
    std::vector<std::string> messages_;
};

}  // namespace editor::ui
