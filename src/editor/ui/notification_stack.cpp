/**
 * @file notification_stack.cpp
 * @brief Implementation of the warning stack
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "notification_stack.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace editor::ui {

namespace {

constexpr int kFontSize = 18;
constexpr float kPadding = 10.0F;
// Gap between the bottom of the window and the newest warning.
constexpr float kBottomMargin = 24.0F;
// Vertical gap between two stacked warnings.
constexpr float kGap = 6.0F;
constexpr float kSideMargin = 12.0F;
constexpr utils::WrappedColor kBackground = {90, 30, 30, 235};
constexpr utils::WrappedColor kBorder = {230, 80, 80, 255};
constexpr utils::WrappedColor kText = {255, 255, 255, 255};

utils::WrappedColor Faded(utils::WrappedColor color, float opacity) {
    color.a_ = static_cast<unsigned char>(
        std::lround(static_cast<float>(color.a_) * opacity));
    return color;
}

}  // namespace

void NotificationStack::Push(std::string text, double now, double duration) {
    entries_.push_back(Entry{
        .text_ = std::move(text),
        .expires_at_ = now + duration,
    });
    while (entries_.size() > kMaxEntries) {
        entries_.pop_front();
    }
}

void NotificationStack::Prune(double now) {
    std::erase_if(entries_, [now](const Entry &entry) {
        return entry.expires_at_ <= now;
    });
}

float NotificationStack::Opacity(const Entry &entry, double now) noexcept {
    const double kRemaining = entry.expires_at_ - now;
    return static_cast<float>(std::clamp(kRemaining / kFadeSeconds, 0.0, 1.0));
}

void NotificationStack::Draw(utils::WrappedVector2 screen, double now) const {
    const float kHeight = static_cast<float>(kFontSize) + 2.0F * kPadding;
    float y = screen.y_ - kBottomMargin - kHeight;

    // Newest at the bottom, older ones above it.
    for (auto it = entries_.rbegin(); it != entries_.rend(); ++it) {
        const float kOpacity = Opacity(*it, now);
        if (kOpacity > 0.0F) {
            const float kWidth =
                utils::MeasureTextWrapped(it->text_.c_str(), kFontSize) +
                2.0F * kPadding;
            const float kX = std::max(kSideMargin, (screen.x_ - kWidth) / 2.0F);

            utils::DrawRectangleWrapped(kX, y, kWidth, kHeight,
                                        Faded(kBackground, kOpacity));
            utils::DrawRectangleLinesWrapped(kX, y, kWidth, kHeight,
                                             Faded(kBorder, kOpacity));
            utils::DrawTextWrapped(it->text_.c_str(), kX + kPadding,
                                   y + kPadding, kFontSize,
                                   Faded(kText, kOpacity));
        }
        y -= kHeight + kGap;
    }
}

}  // namespace editor::ui
