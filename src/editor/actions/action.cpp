/**
 * @file action.cpp
 * @brief Implementation of Action and Shortcut.
 *
 * @author Created by mathys-f
 * @date Created on 06-10-2026
 */

#include "action.hpp"

namespace editor::actions {

std::string Shortcut::ToString() const {
    if (!IsValid()) return "";
    std::string result;
    if (ctrl_) result += "Ctrl+";
    if (shift_) result += "Shift+";
    if (alt_) result += "Alt+";

    // Simple mapping for common keys, could be expanded
    if (key_ >= utils::WrappedKey::kA && key_ <= utils::WrappedKey::kZ) {
        result +=
            static_cast<char>('A' + (static_cast<int>(key_) -
                                     static_cast<int>(utils::WrappedKey::kA)));
    } else {
        result += "Key";
    }
    return result;
}

}  // namespace editor::actions
