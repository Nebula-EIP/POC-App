/**
 * @file notification_stack_test.cpp
 * @brief Tests for the warning stack (no window needed: time is passed in)
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "ui/notification_stack.hpp"

#include <gtest/gtest.h>

#include <string>

using editor::ui::NotificationStack;

TEST(NotificationStack, EmptyByDefault) {
    const NotificationStack kStack;
    EXPECT_TRUE(kStack.Entries().empty());
}

TEST(NotificationStack, NewWarningIsStackedAfterTheOthers) {
    NotificationStack stack;
    stack.Push("first", 0.0);
    stack.Push("second", 1.0);

    ASSERT_EQ(stack.Entries().size(), 2U);
    EXPECT_EQ(stack.Entries()[0].text_, "first");
    EXPECT_EQ(stack.Entries()[1].text_, "second");
}

TEST(NotificationStack, EachWarningKeepsItsOwnDuration) {
    NotificationStack stack;
    stack.Push("first", 0.0);
    stack.Push("second", 3.0);

    stack.Prune(NotificationStack::kDefaultDuration + 0.1);
    ASSERT_EQ(stack.Entries().size(), 1U);
    EXPECT_EQ(stack.Entries()[0].text_, "second");

    stack.Prune(3.0 + NotificationStack::kDefaultDuration);
    EXPECT_TRUE(stack.Entries().empty());
}

TEST(NotificationStack, IdenticalWarningsAreStackedToo) {
    NotificationStack stack;
    stack.Push("same", 0.0);
    stack.Push("same", 0.5);

    EXPECT_EQ(stack.Entries().size(), 2U);
}

TEST(NotificationStack, DropsTheOldestBeyondTheLimit) {
    NotificationStack stack;
    for (std::size_t i = 0; i <= NotificationStack::kMaxEntries; ++i) {
        stack.Push("warning " + std::to_string(i), 0.0);
    }

    ASSERT_EQ(stack.Entries().size(), NotificationStack::kMaxEntries);
    EXPECT_EQ(stack.Entries().front().text_, "warning 1");
    EXPECT_EQ(stack.Entries().back().text_,
              "warning " + std::to_string(NotificationStack::kMaxEntries));
}

TEST(NotificationStack, FadesOutAtTheEnd) {
    NotificationStack stack;
    stack.Push("warning", 0.0, 4.0);
    const NotificationStack::Entry &kEntry = stack.Entries().front();

    EXPECT_FLOAT_EQ(NotificationStack::Opacity(kEntry, 0.0), 1.0F);
    EXPECT_FLOAT_EQ(NotificationStack::Opacity(kEntry, 3.0), 1.0F);
    EXPECT_NEAR(NotificationStack::Opacity(
                    kEntry, 4.0 - NotificationStack::kFadeSeconds / 2.0),
                0.5F, 0.001F);
    EXPECT_FLOAT_EQ(NotificationStack::Opacity(kEntry, 4.0), 0.0F);
    EXPECT_FLOAT_EQ(NotificationStack::Opacity(kEntry, 10.0), 0.0F);
}
