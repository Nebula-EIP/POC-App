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
#include <string>
#include <utility>
#include <vector>

namespace editor::ui {

namespace {

constexpr int kFontSize = 18;
constexpr float kPadding = 10.0F;
// Gap between the bottom of the window and the newest warning.
constexpr float kBottomMargin = 24.0F;
// Vertical gap between two stacked warnings.
constexpr float kGap = 6.0F;
constexpr float kSideMargin = 12.0F;
// Space between two lines of a long warning.
constexpr float kLineGap = 4.0F;
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

std::vector<std::string> NotificationStack::WrapText(
    const std::string &text, float max_width,
    const std::function<float(const std::string &)> &measure) {
    std::vector<std::string> lines;
    std::string line;
    std::size_t start = 0;
    while (start <= text.size()) {
        std::size_t end = text.find(' ', start);
        if (end == std::string::npos) end = text.size();
        const std::string kWord = text.substr(start, end - start);
        const std::string kCandidate =
            line.empty() ? kWord : line + " " + kWord;
        if (!line.empty() && measure(kCandidate) > max_width) {
            lines.push_back(line);
            line = kWord;
        } else {
            line = kCandidate;
        }
        start = end + 1;
    }
    lines.push_back(line);
    return lines;
}

void NotificationStack::Draw(utils::WrappedVector2 screen, double now) const {
    const auto kMeasure = [](const std::string &text) {
        return utils::MeasureTextWrapped(text.c_str(), kFontSize);
    };
    const float kMaxTextWidth =
        std::max(screen.x_ - 2.0F * (kSideMargin + kPadding), 50.0F);
    float bottom = screen.y_ - kBottomMargin;

    // Newest at the bottom, older ones above it.
    for (auto it = entries_.rbegin(); it != entries_.rend(); ++it) {
        const std::vector<std::string> kLines =
            WrapText(it->text_, kMaxTextWidth, kMeasure);
        float text_width = 0.0F;
        for (const std::string &line : kLines) {
            text_width = std::max(text_width, kMeasure(line));
        }
        const auto kLineCount = static_cast<float>(kLines.size());
        const float kHeight = kLineCount * static_cast<float>(kFontSize) +
                              (kLineCount - 1.0F) * kLineGap + 2.0F * kPadding;
        const float kY = bottom - kHeight;

        const float kOpacity = Opacity(*it, now);
        if (kOpacity > 0.0F) {
            const float kWidth = text_width + 2.0F * kPadding;
            const float kX = std::max(kSideMargin, (screen.x_ - kWidth) / 2.0F);

            utils::DrawRectangleWrapped(kX, kY, kWidth, kHeight,
                                        Faded(kBackground, kOpacity));
            utils::DrawRectangleLinesWrapped(kX, kY, kWidth, kHeight,
                                             Faded(kBorder, kOpacity));
            float line_y = kY + kPadding;
            for (const std::string &line : kLines) {
                utils::DrawTextWrapped(line.c_str(), kX + kPadding, line_y,
                                       kFontSize, Faded(kText, kOpacity));
                line_y += static_cast<float>(kFontSize) + kLineGap;
            }
        }
        bottom = kY - kGap;
    }
}

}  // namespace editor::ui
