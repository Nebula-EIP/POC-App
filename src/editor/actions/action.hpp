/**
 * @file action.hpp
 * @brief Action and Shortcut structures.
 *
 * @author Created by mathys-f
 * @date Created on 06-10-2026
 */

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

    std::string ToString() const;
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
