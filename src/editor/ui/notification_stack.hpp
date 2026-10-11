/**
 * @file notification_stack.hpp
 * @brief Stack of warning messages shown at the bottom center of the window
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#pragma once

#include <cstddef>
#include <deque>
#include <string>

#include "utils/raylib_wrapper.hpp"

namespace editor::ui {

/**
 * @brief Warnings stacked at the bottom center of the window.
 *
 * Each warning lives for its own duration: a new one never replaces the
 * others, it appears at the bottom and pushes the older ones up. A warning
 * fades out during its last kFadeSeconds. When more than kMaxEntries are
 * shown, the oldest is dropped.
 *
 * Time is always passed in (seconds, e.g. utils::GetTimeWrapped()), so the
 * stack is testable without a window.
 */
class NotificationStack final {
   public:
    static constexpr double kDefaultDuration = 4.0;
    static constexpr double kFadeSeconds = 0.5;
    static constexpr std::size_t kMaxEntries = 5;

    struct Entry {
        std::string text_;
        double expires_at_;
    };

    /**
     * @brief Adds a warning below the ones already shown.
     *
     * @param text The message.
     * @param now The current time, in seconds.
     * @param duration How long the warning stays, in seconds.
     */
    void Push(std::string text, double now, double duration = kDefaultDuration);

    /**
     * @brief Removes the warnings whose duration is over.
     *
     * @param now The current time, in seconds.
     */
    void Prune(double now);

    /**
     * @brief Gets the warnings, oldest first.
     */
    [[nodiscard]] const std::deque<Entry> &Entries() const noexcept {
        return entries_;
    }

    /**
     * @brief Opacity of a warning: 1, then down to 0 during its last
     * kFadeSeconds.
     *
     * @param entry The warning.
     * @param now The current time, in seconds.
     *
     * @return The opacity, in [0, 1].
     */
    [[nodiscard]] static float Opacity(const Entry &entry, double now) noexcept;

    /**
     * @brief Draws the warnings, newest at the bottom center of the screen.
     * Must be called in screen space (outside the camera).
     *
     * @param screen The screen size, in pixels.
     * @param now The current time, in seconds.
     */
    void Draw(utils::WrappedVector2 screen, double now) const;

   private:
    std::deque<Entry> entries_;
};

}  // namespace editor::ui
