/**
 * @file top_bar_test.cpp
 * @brief Unit tests for the TopBar class and the Menu builder.
 *
 * No window is opened: input is fed through TopBarInput and text is measured
 * with a fixed width per character.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 29-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 29-09-2026
 */

#include "ui/top_bar.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using editor::ui::Menu;
using editor::ui::TopBar;
using editor::ui::TopBarInput;
using utils::WrappedRectangle;
using utils::WrappedVector2;

constexpr float kScreenWidth = 800.0F;
constexpr WrappedVector2 kCanvasPoint{400.0F, 400.0F};

float FixedWidth(const std::string &text, int /*font_size*/) {
    return 8.0F * static_cast<float>(text.size());
}

WrappedVector2 Center(WrappedRectangle rectangle) {
    return {rectangle.x_ + rectangle.width_ / 2.0F,
            rectangle.y_ + rectangle.height_ / 2.0F};
}

TopBarInput Hover(WrappedVector2 cursor) {
    TopBarInput input;
    input.cursor_ = cursor;
    input.screen_width_ = kScreenWidth;
    return input;
}

TopBarInput Press(WrappedVector2 cursor) {
    TopBarInput input = Hover(cursor);
    input.left_pressed_ = true;
    input.any_down_ = true;
    return input;
}

TopBarInput Hold(WrappedVector2 cursor) {
    TopBarInput input = Hover(cursor);
    input.any_down_ = true;
    return input;
}

/// Menus used by the tests:
///   0 "File": "New" (not implemented), separator, "Quit"
///   1 "View": "Recenter", "Zoom"
class TopBarTest : public ::testing::Test {
   protected:
    void SetUp() override {
        bar_.AddMenu("File").AddAction("New").AddSeparator().AddAction(
            "Quit", [this] { ++quit_; });
        bar_.AddMenu("View")
            .AddAction("Recenter", [this] { ++recenter_; })
            .AddAction("Zoom", [this] { ++zoom_; });
        bar_.Update(Hover(kCanvasPoint));
    }

    /// Clicks on the title of a menu, then releases.
    void ClickTitle(std::size_t menu) {
        bar_.Update(Press(Center(bar_.TitleRect(menu))));
        bar_.Update(Hover(Center(bar_.TitleRect(menu))));
    }

    /// Clicks on an item of the open menu, then releases.
    bool ClickItem(std::size_t item) {
        const WrappedVector2 kPoint = Center(bar_.ItemRect(item));
        const bool kCaptured = bar_.Update(Press(kPoint));
        bar_.Update(Hover(kPoint));
        return kCaptured;
    }

    TopBar bar_{editor::ui::TopBarStyle{}, FixedWidth};
    int quit_ = 0;
    int recenter_ = 0;
    int zoom_ = 0;
};

}  // namespace

// ---------------------------------------------------------------------------
// Menu builder

TEST(MenuTest, BuilderAppendsItemsInOrder) {
    Menu menu("Help");
    menu.AddAction("Docs", [] {}).AddSeparator().AddAction("About");

    ASSERT_EQ(menu.Items().size(), 3U);
    EXPECT_EQ(menu.Title(), "Help");
    EXPECT_EQ(menu.Items()[0].label_, "Docs");
    EXPECT_TRUE(menu.Items()[1].separator_);
    EXPECT_EQ(menu.Items()[2].label_, "About");
}

TEST(MenuTest, OnlyActionsWithACommandAreEnabled) {
    Menu menu("Help");
    menu.AddAction("Docs", [] {}).AddSeparator().AddAction("About");

    EXPECT_TRUE(menu.Items()[0].IsEnabled());
    EXPECT_FALSE(menu.Items()[1].IsEnabled());
    EXPECT_FALSE(menu.Items()[2].IsEnabled());
}

TEST_F(TopBarTest, NewMenusGoToTheRightOfTheOthers) {
    bar_.AddMenu("Help").AddAction("About");

    ASSERT_EQ(bar_.Menus().size(), 3U);
    EXPECT_EQ(bar_.TitleRect(0).x_, 0.0F);
    EXPECT_GE(bar_.TitleRect(2).x_,
              bar_.TitleRect(1).x_ + bar_.TitleRect(1).width_);
}

// ---------------------------------------------------------------------------
// Opening and closing

TEST_F(TopBarTest, ClickOnTitleOpensAndSecondClickCloses) {
    ClickTitle(1);
    EXPECT_EQ(bar_.OpenMenu(), 1U);

    ClickTitle(1);
    EXPECT_FALSE(bar_.IsOpen());
}

TEST_F(TopBarTest, ClickOutsideClosesTheMenu) {
    ClickTitle(0);

    EXPECT_TRUE(bar_.Update(Press(kCanvasPoint)));
    EXPECT_FALSE(bar_.IsOpen());
}

TEST_F(TopBarTest, ClickOnEmptyPartOfTheBarClosesTheMenu) {
    ClickTitle(0);

    EXPECT_TRUE(bar_.Update(Press({kScreenWidth - 10.0F, 10.0F})));
    EXPECT_FALSE(bar_.IsOpen());
}

TEST_F(TopBarTest, EscapeClosesTheMenu) {
    ClickTitle(1);
    TopBarInput input = Hover(kCanvasPoint);
    input.escape_pressed_ = true;

    EXPECT_TRUE(bar_.Update(input));
    EXPECT_FALSE(bar_.IsOpen());
}

TEST_F(TopBarTest, HoveringAnotherTitleSwitchesTheOpenMenu) {
    ClickTitle(0);

    bar_.Update(Hover(Center(bar_.TitleRect(1))));
    EXPECT_EQ(bar_.OpenMenu(), 1U);
}

TEST_F(TopBarTest, HoveringATitleDoesNotOpenAClosedMenu) {
    bar_.Update(Hover(Center(bar_.TitleRect(1))));
    EXPECT_FALSE(bar_.IsOpen());
}

// ---------------------------------------------------------------------------
// Items

TEST_F(TopBarTest, ClickOnAnItemRunsItsCommandOnceAndCloses) {
    ClickTitle(0);

    EXPECT_TRUE(ClickItem(2));
    EXPECT_EQ(quit_, 1);
    EXPECT_FALSE(bar_.IsOpen());
}

TEST_F(TopBarTest, NotImplementedItemDoesNothingAndKeepsTheMenuOpen) {
    ClickTitle(0);

    EXPECT_TRUE(ClickItem(0));
    EXPECT_TRUE(ClickItem(1));  // separator
    EXPECT_TRUE(bar_.IsOpen());
    EXPECT_EQ(quit_, 0);
}

TEST_F(TopBarTest, EachItemRunsItsOwnCommand) {
    ClickTitle(1);
    ClickItem(1);

    EXPECT_EQ(zoom_, 1);
    EXPECT_EQ(recenter_, 0);
}

// ---------------------------------------------------------------------------
// Mouse capture: what must not reach the canvas

TEST_F(TopBarTest, ClickOnTheBarIsCaptured) {
    EXPECT_TRUE(bar_.Update(Press({kScreenWidth - 10.0F, 5.0F})));
    EXPECT_TRUE(bar_.Update(Press(Center(bar_.TitleRect(0)))));
}

TEST_F(TopBarTest, HoveringTheBarIsCapturedButNotTheCanvas) {
    EXPECT_TRUE(bar_.Update(Hover({kScreenWidth - 10.0F, 5.0F})));
    EXPECT_FALSE(bar_.Update(Hover(kCanvasPoint)));
}

TEST_F(TopBarTest, EverythingIsCapturedWhileAMenuIsOpen) {
    ClickTitle(0);

    EXPECT_TRUE(bar_.Update(Hover(kCanvasPoint)));
}

TEST_F(TopBarTest, ClickThatClosesAMenuStaysCapturedUntilRelease) {
    ClickTitle(0);

    EXPECT_TRUE(bar_.Update(Press(kCanvasPoint)));
    EXPECT_TRUE(bar_.Update(Hold(kCanvasPoint)));
    bar_.Update(Hover(kCanvasPoint));  // release
    EXPECT_FALSE(bar_.Update(Hover(kCanvasPoint)));
}

TEST_F(TopBarTest, CanvasDragCrossingTheBarIsNotCaptured) {
    EXPECT_FALSE(bar_.Update(Press(kCanvasPoint)));
    EXPECT_FALSE(bar_.Update(Hold(Center(bar_.TitleRect(1)))));
    EXPECT_FALSE(bar_.IsOpen());
}

// ---------------------------------------------------------------------------
// Layout

TEST_F(TopBarTest, TitlesStayInsideTheBar) {
    for (std::size_t menu = 0; menu < bar_.Menus().size(); ++menu) {
        EXPECT_EQ(bar_.TitleRect(menu).y_, 0.0F);
        EXPECT_EQ(bar_.TitleRect(menu).height_, bar_.Height());
    }
}

TEST_F(TopBarTest, MenuOpensUnderItsTitle) {
    ClickTitle(1);

    EXPECT_GE(bar_.ItemRect(0).y_, bar_.Height());
    EXPECT_EQ(bar_.ItemRect(0).x_, bar_.TitleRect(1).x_);
}

TEST_F(TopBarTest, MenuStaysInsideANarrowWindow) {
    ClickTitle(1);
    TopBarInput input = Hover(kCanvasPoint);
    // Room for the menu, but not from where its title is.
    input.screen_width_ = bar_.TitleRect(1).x_ + 150.0F;
    bar_.Update(input);

    const WrappedRectangle kItem = bar_.ItemRect(0);
    ASSERT_GT(kItem.width_, 150.0F);
    ASSERT_LE(kItem.width_, input.screen_width_);
    EXPECT_LE(kItem.x_ + kItem.width_, input.screen_width_);
    EXPECT_GE(kItem.x_, 0.0F);
}
