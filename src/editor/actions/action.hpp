#pragma once

#include <functional>
#include <string>

#include "utils/raylib_wrapper.hpp"

namespace editor::actions {

using ActionCallback = std::function<void()>;

/**
 * @brief Represents a keyboard shortcut.
 */
struct Shortcut {
    utils::WrappedKey key_ = utils::WrappedKey::kNone;
    bool ctrl_ = false;
    bool shift_ = false;
    bool alt_ = false;

    bool IsValid() const noexcept { return key_ != utils::WrappedKey::kNone; }

    std::string ToString() const {
        if (!IsValid()) return "";
        std::string result;
        if (ctrl_) result += "Ctrl+";
        if (shift_) result += "Shift+";
        if (alt_) result += "Alt+";

        // Simple mapping for common keys, could be expanded
        if (key_ >= utils::WrappedKey::kA && key_ <= utils::WrappedKey::kZ) {
            result += static_cast<char>(
                'A' + (static_cast<int>(key_) -
                       static_cast<int>(utils::WrappedKey::kA)));
        } else {
            result += "Key";
        }
        return result;
    }
};

/**
 * @brief Represents an action that can be triggered by a shortcut, a menu, or
 * the toolbar.
 */
struct Action {
    std::string id_;
    std::string display_name_;
    Shortcut shortcut_;
    ActionCallback callback_;
};

}  // namespace editor::actions
