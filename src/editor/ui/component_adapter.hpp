/**
 * @file component_adapter.hpp
 * @brief Draws the components a module describes for a node and turns user
 * input on them into the components' callbacks.
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-11-2026
 */

#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/graph/datatypes.hpp"
#include "core/modules/capabilities/renderer_capability.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief Input of one frame for the component fields.
 *
 * The cursor is in the same space as the bounds given to the adapter (world
 * space when the fields are drawn inside the nodes). Keys are true on the
 * frame they are pressed and while they repeat.
 */
struct ComponentInput {
    utils::WrappedVector2 cursor_{0.0F, 0.0F};
    bool left_pressed_ = false;
    bool left_down_ = false;
    /// left_pressed_ is the second click of a double click.
    bool double_click_ = false;
    std::string typed_;  ///< UTF-8 text typed this frame
    bool backspace_ = false;
    bool delete_ = false;
    bool enter_ = false;
    bool escape_ = false;
    bool left_ = false;   ///< Left arrow
    bool right_ = false;  ///< Right arrow
    bool home_ = false;
    bool end_ = false;
    /// Shift held: caret moves and clicks extend the selection.
    bool shift_ = false;
    /// Ctrl held (not AltGr): arrows, Backspace and Delete work by word.
    bool control_ = false;
    bool select_all_ = false;  ///< Ctrl+A
    bool copy_ = false;        ///< Ctrl+C
    bool cut_ = false;         ///< Ctrl+X
    bool paste_ = false;       ///< Ctrl+V
    double time_ = 0.0;        ///< Seconds, makes the caret blink

    /**
     * @brief Reads this frame's mouse and keyboard; drains the typed
     * characters. Call it once per frame (double clicks are detected here).
     *
     */
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
 *
 * The edited field works like a standard single-line text input: a caret and
 * a selection placed with the mouse (click, drag, double click selects all)
 * and the keyboard (arrows, Home, End, Ctrl for words, Shift to select,
 * Ctrl+A/C/X/V). Caret and selection positions count UTF-8 characters, not
 * bytes. Long texts scroll so that the caret stays visible.
 */
class ComponentAdapter final {
   public:
    /**
     * @brief Measures the width of a text drawn in a field, in the units of
     * the bounds given to the adapter.
     *
     */
    using TextMeasurer = std::function<float(const std::string &)>;

    /**
     * @brief Access to the clipboard used by Ctrl+C, Ctrl+X and Ctrl+V.
     *
     */
    struct Clipboard {
        std::function<std::string()> read_;
        std::function<void(const std::string &)> write_;
    };

    /**
     * @brief Creates the adapter.
     *
     * @param measurer Measures field texts. Defaults to the editor font;
     * without a window, to the estimate used for the layout.
     * @param clipboard Defaults to the system clipboard; without a window,
     * an empty one.
     */
    explicit ComponentAdapter(TextMeasurer measurer = {},
                              Clipboard clipboard = {});

    /**
     * @brief Height of one component row.
     *
     */
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
     * list), or while the mouse selects text in a field: nothing else should
     * use the pointer.
     */
    bool Update(core::NodeId node_id,
                const core::capa::ComponentList &components,
                utils::WrappedRectangle bounds, const ComponentInput &input);

    /**
     * @brief Draws the components.
     *
     * @param active true when the node can be edited (it is selected).
     * @param pixel_size Size of one screen pixel in the units of the bounds
     * (1 / zoom), so the caret stays visible at any zoom.
     */
    void Draw(core::NodeId node_id, const core::capa::ComponentList &components,
              utils::WrappedRectangle bounds, bool active,
              float pixel_size = 1.0F) const;

    /**
     * @brief True while a text or number field has the keyboard.
     *
     */
    [[nodiscard]] bool IsEditingText() const noexcept;

    /**
     * @brief True while a select list is open.
     *
     */
    [[nodiscard]] bool IsListOpen() const noexcept;

    /**
     * @brief Node of the focused field or open list, if any.
     *
     */
    [[nodiscard]] std::optional<core::NodeId> FocusedNode() const noexcept;

    /**
     * @brief Text being typed in the focused field (empty when none).
     *
     */
    [[nodiscard]] const std::string &EditedText() const noexcept;

    /**
     * @brief Explanation of why the typed text is invalid (empty when valid).
     *
     */
    [[nodiscard]] const std::string &EditError() const noexcept;

    /**
     * @brief Caret position in the edited text, in UTF-8 characters (0 when
     * nothing is edited).
     *
     */
    [[nodiscard]] std::size_t Caret() const noexcept;

    /**
     * @brief Selected characters of the edited text: [first, second), in
     * UTF-8 characters. Empty (first == second == caret) when nothing is
     * selected.
     *
     */
    [[nodiscard]] std::pair<std::size_t, std::size_t> Selection()
        const noexcept;

    /**
     * @brief Text of the selection (empty when nothing is selected).
     *
     */
    [[nodiscard]] std::string SelectedText() const;

    /**
     * @brief First character drawn in the edited field: above 0 when a long
     * text is scrolled to show the caret.
     *
     */
    [[nodiscard]] std::size_t FirstVisibleCharacter() const noexcept;

    /**
     * @brief Drops the current edit and closes any list, without applying.
     *
     */
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

    /**
     * @brief Text shown for a number (no decimals for an integer field).
     *
     */
    static std::string FormatNumber(double value, bool integer);

   private:
    struct Edit {
        core::NodeId node_id_ = 0;
        core::capa::ComponentId component_id_ = 0;
        std::string text_;
        std::string error_;
        std::size_t caret_ = 0;    ///< In UTF-8 characters
        std::size_t anchor_ = 0;   ///< Other end of the selection
        std::size_t scroll_ = 0;   ///< First character drawn
        bool dragging_ = false;    ///< The mouse is selecting text
        double caret_time_ = 0.0;  ///< Last caret move, restarts the blink
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
    bool HandleEditInput(const core::capa::ComponentList &components,
                         utils::WrappedRectangle bounds, std::size_t index,
                         const ComponentInput &input);
    bool HandleClick(core::NodeId node_id,
                     const core::capa::ComponentList &components,
                     utils::WrappedRectangle bounds,
                     const ComponentInput &input);
    void ClickField(core::NodeId node_id,
                    const core::capa::ComponentList &components,
                    utils::WrappedRectangle bounds, std::size_t index,
                    const ComponentInput &input);
    bool EditText(const ComponentInput &input);
    void Insert(const std::string &text);
    void EraseRange(std::size_t first, std::size_t last);
    bool EraseSelection();
    void MoveCaret(std::size_t position, bool extend);
    void KeepCaretVisible(const core::capa::ComponentList &components,
                          utils::WrappedRectangle bounds);

    /// Width of the characters [first, last) of a text.
    float Span(const std::string &text, std::size_t first,
               std::size_t last) const;
    /// Distance from the start of character first to the caret before last.
    float CaretOffset(const std::string &text, std::size_t first,
                      std::size_t last) const;
    /// Last character boundary drawn from first within a width.
    std::size_t LastVisible(const std::string &text, std::size_t first,
                            float width) const;
    /// Character boundary under a point of a field being edited.
    std::size_t IndexAt(utils::WrappedRectangle control,
                        utils::WrappedVector2 point, bool dragging) const;
    void DrawValue(utils::WrappedRectangle box, const std::string &text,
                   utils::WrappedColor color) const;
    void DrawEditedText(utils::WrappedRectangle control,
                        float pixel_size) const;
    void DrawRow(const core::capa::ComponentList &components,
                 utils::WrappedRectangle bounds, std::size_t index,
                 core::NodeId node_id, bool active, float pixel_size) const;
    void DrawOverlay(core::NodeId node_id,
                     const core::capa::ComponentList &components,
                     utils::WrappedRectangle bounds) const;

    TextMeasurer measurer_;
    Clipboard clipboard_;
    double now_ = 0.0;  ///< Time of the last update, for the caret blink
    std::optional<Edit> edit_;
    std::optional<OpenList> open_list_;
    std::vector<std::string> messages_;
};

}  // namespace editor::ui
