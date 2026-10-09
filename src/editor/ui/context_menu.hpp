/**
 * @file context_menu.hpp
 * @brief ContextMenu UI component.
 *
 * @author Created by mathys-f
 * @date Created on 06-10-2026
 */

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "actions/action.hpp"
#include "top_bar.hpp"  // Reuse TopBarStyle maybe?
#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief An entry in the context menu.
 */
struct ContextMenuItem {
    std::string label_;
    actions::ActionCallback command_;
    std::string shortcut_text_;
    bool separator_ = false;

    [[nodiscard]] bool IsEnabled() const;
};

/**
 * @brief Input state for the context menu.
 */
struct ContextMenuInput {
    utils::WrappedVector2 cursor_{0.0F, 0.0F};
    bool left_pressed_ = false;
    bool right_pressed_ = false;
    bool escape_pressed_ = false;
    float screen_width_ = 0.0F;
    float screen_height_ = 0.0F;
};

/**
 * @brief A floating context menu that appears at the cursor position.
 */
class ContextMenu final {
   public:
    using TextMeasurer = std::function<float(const std::string &, int)>;

    explicit ContextMenu(TopBarStyle style = {}, TextMeasurer measurer = {});

    /**
     * @brief Adds an action to the menu.
     */
    ContextMenu &AddAction(const std::string &label,
                           actions::ActionCallback command,
                           const std::string &shortcut = "");

    /**
     * @brief Adds a separator.
     */
    ContextMenu &AddSeparator();

    /**
     * @brief Clears the menu.
     */
    void Clear();

    /**
     * @brief Opens the menu at the specified screen position.
     * Clamps the position so the menu remains on screen.
     */
    void OpenAt(utils::WrappedVector2 position, float screen_width,
                float screen_height);

    /**
     * @brief Closes the menu.
     */
    void Close() noexcept { is_open_ = false; }

    [[nodiscard]] bool IsOpen() const noexcept { return is_open_; }

    /**
     * @brief Handles input.
     * @return true if the menu consumed the input.
     */
    bool Update(const ContextMenuInput &input);

    /**
     * @brief Draws the menu.
     */
    void Draw() const;

   private:
    [[nodiscard]] utils::WrappedRectangle Bounds() const;
    [[nodiscard]] float ItemHeight(const ContextMenuItem &item) const;
    [[nodiscard]] float MenuWidth() const;
    [[nodiscard]] std::optional<std::size_t> HitTest(
        utils::WrappedVector2 point) const;

    TopBarStyle style_;
    TextMeasurer measurer_;
    std::vector<ContextMenuItem> items_;

    bool is_open_ = false;
    utils::WrappedVector2 position_{0.0F, 0.0F};
    std::optional<std::size_t> hovered_item_;
};

}  // namespace editor::ui
