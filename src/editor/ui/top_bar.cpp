#include "ui/top_bar.hpp"

#include <algorithm>

namespace editor_ui {

namespace {
float GetTextWidth(const std::string &text, int font_size) {
    return static_cast<float>(MeasureText(text.c_str(), font_size));
}
}  // namespace

TopBar::TopBar(std::vector<Menu> menus, TopBarStyle style)
    : menus_(std::move(menus)), style_(style) {}

void TopBar::SetMenus(std::vector<Menu> menus) {
    menus_ = std::move(menus);
    CloseMenu();
}

void TopBar::Update() {
    utils::WrappedVector2 mouse = utils::GetCursorPositionWrapped();
    hovered_menu_ = GetMenuUnderMouse(mouse);
    hovered_item_ = GetItemUnderMouse(mouse);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (hovered_item_ >= 0) {
            ActivateItem(open_menu_, hovered_item_);
            CloseMenu();
            return;
        }

        if (hovered_menu_ >= 0) {
            if (open_menu_ == hovered_menu_) {
                CloseMenu();
            } else {
                OpenMenu(hovered_menu_);
            }
            return;
        }

        if (open_menu_ >= 0) {
            CloseMenu();
        }
    }
}

void TopBar::Draw() const {
    const int screen_width = GetScreenWidth();
    utils::DrawRectangleWrapped(0.0f, 0.0f, static_cast<float>(screen_width),
                                style_.bar_height, style_.bar_color);
    utils::DrawRectangleLinesWrapped(0.0f, 0.0f,
                                     static_cast<float>(screen_width),
                                     style_.bar_height, style_.border_color);

    float x = 0.0f;
    for (size_t index = 0; index < menus_.size(); ++index) {
        const Menu &menu = menus_[index];
        const float width = GetMenuWidth(menu);
        const utils::WrappedRectangle menu_rect = {x, 0.0f, width,
                                                    style_.bar_height};

        if (static_cast<int>(index) == hovered_menu_ ||
            static_cast<int>(index) == open_menu_) {
            utils::DrawRectangleWrapped(menu_rect.x, menu_rect.y,
                                        menu_rect.width, menu_rect.height,
                                        style_.hover_color);
        }

        utils::DrawTextWrapped(menu.label.c_str(),
                               menu_rect.x + style_.horizontal_padding,
                               (style_.bar_height - style_.font_size) * 0.5f,
                               style_.font_size, style_.text_color);
        x += width;
    }

    if (open_menu_ < 0 || open_menu_ >= static_cast<int>(menus_.size())) {
        return;
    }

    const Menu &menu = menus_[open_menu_];
    const utils::WrappedRectangle dropdown_rect = GetDropdownRect(open_menu_);

    utils::DrawRectangleWrapped(dropdown_rect.x, dropdown_rect.y,
                                dropdown_rect.width, dropdown_rect.height,
                                style_.menu_color);
    utils::DrawRectangleLinesWrapped(dropdown_rect.x, dropdown_rect.y,
                                     dropdown_rect.width, dropdown_rect.height,
                                     style_.border_color);

    for (size_t item_index = 0; item_index < menu.items.size(); ++item_index) {
        const utils::WrappedRectangle item_rect = {
            dropdown_rect.x,
            dropdown_rect.y + static_cast<float>(item_index) * style_.item_height,
            dropdown_rect.width, style_.item_height};

        if (static_cast<int>(item_index) == hovered_item_) {
            utils::DrawRectangleWrapped(item_rect.x, item_rect.y,
                                        item_rect.width, item_rect.height,
                                        style_.hover_color);
        }

        utils::DrawTextWrapped(menu.items[item_index].label.c_str(),
                               item_rect.x + style_.item_padding,
                               item_rect.y + (style_.item_height - style_.font_size) * 0.5f,
                               style_.font_size, style_.text_color);
    }
}

bool TopBar::BlocksGraphInput() const noexcept {
    return open_menu_ >= 0 || hovered_menu_ >= 0 || hovered_item_ >= 0;
}

float TopBar::GetMenuWidth(const Menu &menu) const {
    float width = GetTextWidth(menu.label, style_.font_size) +
                  style_.horizontal_padding * 2.0f;
    width = std::max(width, style_.minimum_menu_width);
    for (const MenuItem &item : menu.items) {
        width = std::max(width,
                         GetTextWidth(item.label, style_.font_size) +
                             style_.item_padding * 2.0f);
    }
    return width;
}

float TopBar::GetDropdownWidth(const Menu &menu) const {
    return GetMenuWidth(menu);
}

float TopBar::GetMenuX(int menu_index) const {
    float x = 0.0f;
    for (int index = 0; index < menu_index && index < static_cast<int>(menus_.size());
         ++index) {
        x += GetMenuWidth(menus_[index]);
    }
    return x;
}

int TopBar::GetMenuUnderMouse(utils::WrappedVector2 mouse) const {
    if (mouse.y < 0.0f || mouse.y > style_.bar_height) {
        return -1;
    }

    float x = 0.0f;
    for (size_t index = 0; index < menus_.size(); ++index) {
        const float width = GetMenuWidth(menus_[index]);
        if (mouse.x >= x && mouse.x <= x + width) {
            return static_cast<int>(index);
        }
        x += width;
    }

    return -1;
}

int TopBar::GetItemUnderMouse(utils::WrappedVector2 mouse) const {
    if (open_menu_ < 0 || open_menu_ >= static_cast<int>(menus_.size())) {
        return -1;
    }

    const utils::WrappedRectangle dropdown_rect = GetDropdownRect(open_menu_);
    if (!utils::CheckCollisionPointRecWrapped(mouse, dropdown_rect)) {
        return -1;
    }

    const int item_index = static_cast<int>((mouse.y - dropdown_rect.y) /
                                            style_.item_height);
    if (item_index < 0 || item_index >= static_cast<int>(menus_[open_menu_].items.size())) {
        return -1;
    }

    return item_index;
}

utils::WrappedRectangle TopBar::GetMenuRect(int menu_index) const {
    const float x = GetMenuX(menu_index);
    return {x, 0.0f, GetMenuWidth(menus_[menu_index]), style_.bar_height};
}

utils::WrappedRectangle TopBar::GetDropdownRect(int menu_index) const {
    const Menu &menu = menus_[menu_index];
    const float width = GetDropdownWidth(menu);
    const float height = static_cast<float>(menu.items.size()) * style_.item_height;
    float x = GetMenuX(menu_index);
    const float screen_width = static_cast<float>(GetScreenWidth());
    if (x + width > screen_width) {
        x = std::max(0.0f, screen_width - width);
    }
    return {x, style_.bar_height, width, height};
}

void TopBar::CloseMenu() {
    open_menu_ = -1;
    hovered_item_ = -1;
}

void TopBar::OpenMenu(int menu_index) {
    if (menu_index < 0 || menu_index >= static_cast<int>(menus_.size())) {
        CloseMenu();
        return;
    }

    open_menu_ = menu_index;
}

void TopBar::ActivateItem(int menu_index, int item_index) {
    if (menu_index < 0 || menu_index >= static_cast<int>(menus_.size())) {
        return;
    }

    const Menu &menu = menus_[menu_index];
    if (item_index < 0 || item_index >= static_cast<int>(menu.items.size())) {
        return;
    }

    const MenuItem &item = menu.items[item_index];
    if (item.on_click) {
        item.on_click();
    }
}

}  // namespace editor_ui