#include "action_manager.hpp"

namespace editor::actions {

void ActionManager::RegisterAction(Action action) {
    actions_[action.id_] = std::move(action);
}

void ActionManager::TriggerAction(const std::string &id) const {
    if (auto it = actions_.find(id); it != actions_.end()) {
        if (it->second.callback_) {
            it->second.callback_();
        }
    }
}

void ActionManager::ProcessShortcuts() const {
    const bool kCtrl =
        utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl) ||
        utils::IsKeyDownWrapped(utils::WrappedKey::kRightControl);
    const bool kShift =
        utils::IsKeyDownWrapped(utils::WrappedKey::kLeftShift) ||
        utils::IsKeyDownWrapped(utils::WrappedKey::kRightShift);
    const bool kAlt = utils::IsKeyDownWrapped(utils::WrappedKey::kLeftAlt) ||
                      utils::IsKeyDownWrapped(utils::WrappedKey::kRightAlt);

    for (const auto &[id, action] : actions_) {
        const auto &shortcut = action.shortcut_;
        if (!shortcut.IsValid()) {
            continue;
        }

        if (shortcut.ctrl_ == kCtrl && shortcut.shift_ == kShift &&
            shortcut.alt_ == kAlt) {
            // Check for AZERTY 'A' which maps to 'Q' (81) in raylib
            bool is_pressed = utils::IsKeyPressedWrapped(shortcut.key_);
            if (shortcut.key_ == utils::WrappedKey::kA && !is_pressed) {
                is_pressed = utils::IsKeyPressedWrapped(utils::WrappedKey::kQ);
            }
            if (shortcut.key_ == utils::WrappedKey::kZ && !is_pressed) {
                // For completeness, AZERTY Z is QWERTY W
                is_pressed = utils::IsKeyPressedWrapped(utils::WrappedKey::kW);
            }

            // Only trigger on the exact frame the main key is pressed
            if (is_pressed) {
                if (action.callback_) {
                    action.callback_();
                }
            }
        }
    }
}

const Action *ActionManager::GetAction(const std::string &id) const {
    if (auto it = actions_.find(id); it != actions_.end()) {
        return &it->second;
    }
    return nullptr;
}

}  // namespace editor::actions
