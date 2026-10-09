/**
 * @file top_bar.hpp
 * @brief Declaration of the TopBar class, the menu bar drawn at the top of the
 * editor window, and of the menus it displays.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 29-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 29-09-2026
 */

#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief Action run when a menu item is clicked.
 *
 * Menu items never call the core directly: they only hold a command provided
 * by the application. An item without a command is drawn greyed out, which is
 * how "not implemented yet" actions are shown.
 */
using Command = std::function<void()>;

/**
 * @brief One entry of a menu: a clickable action, or a separator.
 */
struct MenuItem {
    std::string label_;
    Command command_;
    std::string shortcut_text_;
    bool separator_ = false;
    bool keep_open_ = false;

    /**
     * @brief Whether clicking the item does something: it is an action with a
     * command.
     */
    [[nodiscard]] bool IsEnabled() const {
        return !separator_ && static_cast<bool>(command_);
    }
};

/**
 * @brief A titled list of items, shown as one entry of the top bar.
 *
 * The builder methods return the menu itself so that calls can be chained:
 * @code
 * top_bar.AddMenu("Aide")
 *     .AddAction("Documentation", [this] { OpenDocumentation(); })
 *     .AddSeparator()
 *     .AddAction("A propos");  // no command yet: greyed out
 * @endcode
 */
class Menu {
   public:
    explicit Menu(std::string title);

    /**
     * @brief Appends a clickable entry.
     *
     * @param label The text of the entry.
     * @param command The action to run. Leave it empty for an action that is
     * not implemented yet: the entry is then greyed out.
     */
    Menu &AddAction(std::string label, Command command = {},
                    std::string shortcut_text = "", bool keep_open = false);

    /**
     * @brief Appends a horizontal line between two groups of entries.
     */
    Menu &AddSeparator();

    [[nodiscard]] const std::string &Title() const noexcept { return title_; }
    [[nodiscard]] const std::vector<MenuItem> &Items() const noexcept {
        return items_;
    }

   private:
    std::string title_;
    std::vector<MenuItem> items_;
};

/**
 * @brief Mouse and keyboard state the top bar reacts to, for one frame.
 *
 * Kept separate from raylib so that the bar can be tested without a window.
 * Use TopBar::ReadInput() in the application.
 */
struct TopBarInput {
    utils::WrappedVector2 cursor_{0.0F, 0.0F};
    bool left_pressed_ = false;   ///< Left button went down this frame.
    bool other_pressed_ = false;  ///< Right or middle button went down.
    bool any_down_ = false;       ///< A mouse button is held.
    bool escape_pressed_ = false;
    float screen_width_ = 0.0F;
};

/**
 * @brief Sizes and colors of the top bar.
 */
struct TopBarStyle {
    float height_ = 30.0F;
    float title_padding_ = 12.0F;
    float item_height_ = 26.0F;
    float separator_height_ = 9.0F;
    float item_padding_ = 14.0F;
    float min_dropdown_width_ = 180.0F;
    int font_size_ = 16;
    utils::WrappedColor bar_ = {24, 24, 28, 255};
    utils::WrappedColor dropdown_ = {36, 36, 42, 255};
    utils::WrappedColor hover_ = {62, 62, 74, 255};
    utils::WrappedColor border_ = {70, 70, 82, 255};
    utils::WrappedColor text_ = {235, 235, 240, 255};
    utils::WrappedColor disabled_text_ = {100, 100, 110, 255};
};

/**
 * @brief The menu bar drawn at the top of the window.
 *
 * It is drawn in screen space, after the canvas, so pan and zoom never affect
 * it. It spans the whole window width, whatever the window size.
 *
 * Each frame, call Update() before the canvas processes its input: when it
 * returns true the mouse belongs to the bar and must not reach the canvas.
 * Then call Draw() after the canvas has been drawn.
 */
class TopBar final {
   public:
    /// Measures the width in pixels of a text drawn at a given font size.
    using TextMeasurer = std::function<float(const std::string &, int)>;

    /**
     * @param style Sizes and colors.
     * @param measurer Text measuring function. Defaults to raylib's, which
     * needs an open window; tests pass their own.
     */
    explicit TopBar(TopBarStyle style = {}, TextMeasurer measurer = {});

    /**
     * @brief Appends a new menu to the right of the existing ones.
     *
     * @return The new menu, to be filled with its builder methods. The
     * reference is invalidated by the next call to AddMenu().
     */
    Menu &AddMenu(std::string title);

    [[nodiscard]] const std::vector<Menu> &Menus() const noexcept {
        return menus_;
    }

    /**
     * @brief Reads this frame's input from raylib.
     */
    static TopBarInput ReadInput();

    /**
     * @brief Handles one frame of input: opening and closing menus, hover,
     * and running the command of a clicked item.
     *
     * @param input This frame's input.
     *
     * @return true when the mouse belongs to the bar this frame and must not
     * reach the canvas: a menu is open, the cursor is over the bar, or the
     * current click started on the bar.
     */
    bool Update(const TopBarInput &input);

    /**
     * @brief Draws the bar and the open menu, in screen space.
     */
    void Draw() const;

    /**
     * @brief Closes the open menu, if any.
     */
    void Close() noexcept { open_menu_.reset(); }

    [[nodiscard]] bool IsOpen() const noexcept {
        return open_menu_.has_value();
    }
    [[nodiscard]] std::optional<std::size_t> OpenMenu() const noexcept {
        return open_menu_;
    }
    [[nodiscard]] float Height() const noexcept { return style_.height_; }

    /**
     * @brief Screen rectangle of the title of a menu, in the bar.
     */
    [[nodiscard]] utils::WrappedRectangle TitleRect(std::size_t menu) const;

    /**
     * @brief Screen rectangle of an item of the open menu. Empty when no menu
     * is open.
     */
    [[nodiscard]] utils::WrappedRectangle ItemRect(std::size_t item) const;

   private:
    /// What is under the cursor.
    struct Hit {
        bool on_bar_ = false;
        bool in_dropdown_ = false;
        std::optional<std::size_t> title_;
        std::optional<std::size_t> item_;
    };

    [[nodiscard]] utils::WrappedRectangle DropdownRect() const;
    [[nodiscard]] float TitleWidth(const Menu &menu) const;
    [[nodiscard]] float ItemHeight(const MenuItem &item) const;
    [[nodiscard]] Hit HitTest(utils::WrappedVector2 point) const;

    /**
     * @brief Opens or closes menus for a click.
     *
     * @return The command of the clicked item, to run once the frame is
     * handled, or an empty command.
     */
    Command HandleClick(const Hit &hit);

    TopBarStyle style_;
    TextMeasurer measurer_;
    std::vector<Menu> menus_;

    std::optional<std::size_t> open_menu_;
    std::optional<std::size_t> hovered_title_;
    std::optional<std::size_t> hovered_item_;

    /// The current mouse press started on the bar or closed a menu.
    bool pointer_owned_ = false;
    float screen_width_ = 0.0F;
};

}  // namespace editor::ui
