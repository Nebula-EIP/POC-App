#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "ui/top_bar.hpp"

namespace {
using editor_ui::Menu;
using editor_ui::MenuItem;
using editor_ui::TopBar;
using editor_ui::TopBarStyle;

TEST(TopBarTest, DefaultStyleValuesAreStable)
{
    TopBarStyle style;

    EXPECT_FLOAT_EQ(style.bar_height, 34.0f);
    EXPECT_FLOAT_EQ(style.item_height, 28.0f);
    EXPECT_FLOAT_EQ(style.horizontal_padding, 16.0f);
    EXPECT_FLOAT_EQ(style.item_padding, 12.0f);
    EXPECT_FLOAT_EQ(style.minimum_menu_width, 120.0f);
    EXPECT_EQ(style.font_size, 16);
}

TEST(TopBarTest, EmptyTopBarDoesNotBlockGraphInput)
{
    TopBar top_bar({});

    EXPECT_FALSE(top_bar.BlocksGraphInput());
}

TEST(TopBarTest, SetMenusKeepsTopBarIdleUntilInteraction)
{
    TopBar top_bar({});

    std::vector<Menu> menus = {
        {"File", {MenuItem{"New", [] {}}, MenuItem{"Open", [] {}}}},
        {"Edit", {MenuItem{"Undo", [] {}}}},
    };

    top_bar.SetMenus(menus);

    EXPECT_FALSE(top_bar.BlocksGraphInput());
}

TEST(TopBarTest, MenuAndItemTypesStoreLabelsAndCallbacks)
{
    bool clicked = false;

    Menu menu{"File", {MenuItem{"Exit", [&clicked] { clicked = true; }}}};

    EXPECT_EQ(menu.label, "File");
    ASSERT_EQ(menu.items.size(), 1u);
    EXPECT_EQ(menu.items.front().label, "Exit");

    menu.items.front().on_click();
    EXPECT_TRUE(clicked);
}

}  // namespace