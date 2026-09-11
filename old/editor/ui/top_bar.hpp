#pragma once

#include <functional>
#include <string>
#include <vector>

#include "raylib_wrapper.hpp"

namespace editor_ui {

struct MenuItem {
    std::string label;
    std::function<void()> on_click;
};

struct Menu {
    std::string label;
    std::vector<MenuItem> items;
};

struct TopBarStyle {
    float bar_height = 34.0f;
    float item_height = 28.0f;
    float horizontal_padding = 16.0f;
    float item_padding = 12.0f;
    float minimum_menu_width = 120.0f;
    int font_size = 16;
    utils::WrappedColor bar_color = {28, 28, 32, 255};
    utils::WrappedColor menu_color = {36, 36, 42, 255};
    utils::WrappedColor hover_color = {58, 58, 68, 255};
    utils::WrappedColor text_color = {245, 245, 245, 255};
    utils::WrappedColor border_color = {70, 70, 80, 255};
};

class TopBar {
   public:
    TopBar(std::vector<Menu> menus, TopBarStyle style = {});

    void SetMenus(std::vector<Menu> menus);
    void Update();
    void Draw() const;

    bool BlocksGraphInput() const noexcept;

   private:
    float GetMenuWidth(const Menu &menu) const;
    float GetDropdownWidth(const Menu &menu) const;
    float GetMenuX(int menu_index) const;
    int GetMenuUnderMouse(utils::WrappedVector2 mouse) const;
    int GetItemUnderMouse(utils::WrappedVector2 mouse) const;
    utils::WrappedRectangle GetMenuRect(int menu_index) const;
    utils::WrappedRectangle GetDropdownRect(int menu_index) const;
    void CloseMenu();
    void OpenMenu(int menu_index);
    void ActivateItem(int menu_index, int item_index);

    std::vector<Menu> menus_;
    TopBarStyle style_;
    int open_menu_ = -1;
    int hovered_menu_ = -1;
    int hovered_item_ = -1;
};

}  // namespace editor_ui