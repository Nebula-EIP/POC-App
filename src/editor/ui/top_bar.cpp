#include "top_bar.hpp"

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
    const int kScreenWidth = GetScreenWidth();
    utils::DrawRectangleWrapped(0.0f, 0.0f, static_cast<float>(kScreenWidth),
                                style_.bar_height, style_.bar_color);
    utils::DrawRectangleLinesWrapped(0.0f, 0.0f,
                                     static_cast<float>(kScreenWidth),
                                     style_.bar_height, style_.border_color);

    float x = 0.0f;
    for (size_t index = 0; index < menus_.size(); ++index) {
        const Menu &menu = menus_[index];
        const float kWidth = GetMenuWidth(menu);
        const utils::WrappedRectangle kMenuRect = {x, 0.0f, kWidth,
                                                   style_.bar_height};

        if (static_cast<int>(index) == hovered_menu_ ||
            static_cast<int>(index) == open_menu_) {
            utils::DrawRectangleWrapped(kMenuRect.x, kMenuRect.y,
                                        kMenuRect.width, kMenuRect.height,
                                        style_.hover_color);
        }

        utils::DrawTextWrapped(menu.label.c_str(),
                               kMenuRect.x + style_.horizontal_padding,
                               (style_.bar_height - style_.font_size) * 0.5f,
                               style_.font_size, style_.text_color);
        x += kWidth;
    }

    if (open_menu_ < 0 || open_menu_ >= static_cast<int>(menus_.size())) {
        return;
    }

    const Menu &menu = menus_[open_menu_];
    const utils::WrappedRectangle kDropdownRect = GetDropdownRect(open_menu_);

    utils::DrawRectangleWrapped(kDropdownRect.x, kDropdownRect.y,
                                kDropdownRect.width, kDropdownRect.height,
                                style_.menu_color);
    utils::DrawRectangleLinesWrapped(kDropdownRect.x, kDropdownRect.y,
                                     kDropdownRect.width, kDropdownRect.height,
                                     style_.border_color);

    for (size_t item_index = 0; item_index < menu.items.size(); ++item_index) {
        const utils::WrappedRectangle kItemRect = {
            kDropdownRect.x,
            kDropdownRect.y +
                static_cast<float>(item_index) * style_.item_height,
            kDropdownRect.width, style_.item_height};

        if (static_cast<int>(item_index) == hovered_item_) {
            utils::DrawRectangleWrapped(kItemRect.x, kItemRect.y,
                                        kItemRect.width, kItemRect.height,
                                        style_.hover_color);
        }

        utils::DrawTextWrapped(
            menu.items[item_index].label.c_str(),
            kItemRect.x + style_.item_padding,
            kItemRect.y + (style_.item_height - style_.font_size) * 0.5f,
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
        width = std::max(width, GetTextWidth(item.label, style_.font_size) +
                                    style_.item_padding * 2.0f);
    }
    return width;
}

float TopBar::GetDropdownWidth(const Menu &menu) const {
    return GetMenuWidth(menu);
}

float TopBar::GetMenuX(int menu_index) const {
    float x = 0.0f;
    for (int index = 0;
         index < menu_index && index < static_cast<int>(menus_.size());
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
        const float kWidth = GetMenuWidth(menus_[index]);
        if (mouse.x >= x && mouse.x <= x + kWidth) {
            return static_cast<int>(index);
        }
        x += kWidth;
    }

    return -1;
}

int TopBar::GetItemUnderMouse(utils::WrappedVector2 mouse) const {
    if (open_menu_ < 0 || open_menu_ >= static_cast<int>(menus_.size())) {
        return -1;
    }

    const utils::WrappedRectangle kDropdownRect = GetDropdownRect(open_menu_);
    if (!utils::CheckCollisionPointRecWrapped(mouse, kDropdownRect)) {
        return -1;
    }

    const int kItemIndex =
        static_cast<int>((mouse.y - kDropdownRect.y) / style_.item_height);
    if (kItemIndex < 0 ||
        kItemIndex >= static_cast<int>(menus_[open_menu_].items.size())) {
        return -1;
    }

    return kItemIndex;
}

utils::WrappedRectangle TopBar::GetMenuRect(int menu_index) const {
    const float kX = GetMenuX(menu_index);
    return {kX, 0.0f, GetMenuWidth(menus_[menu_index]), style_.bar_height};
}

utils::WrappedRectangle TopBar::GetDropdownRect(int menu_index) const {
    const Menu &menu = menus_[menu_index];
    const float kWidth = GetDropdownWidth(menu);
    const float kHeight =
        static_cast<float>(menu.items.size()) * style_.item_height;
    float x = GetMenuX(menu_index);
    const float kScreenWidth = static_cast<float>(GetScreenWidth());
    if (x + kWidth > kScreenWidth) {
        x = std::max(0.0f, kScreenWidth - kWidth);
    }
    return {x, style_.bar_height, kWidth, kHeight};
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