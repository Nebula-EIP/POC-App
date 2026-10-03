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
    const bool kCtrl = utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl);
    const bool kShift = utils::IsKeyDownWrapped(utils::WrappedKey::kLeftShift);
    const bool kAlt = utils::IsKeyDownWrapped(utils::WrappedKey::kLeftAlt);

    for (const auto &[id, action] : actions_) {
        const auto &shortcut = action.shortcut_;
        if (!shortcut.IsValid()) {
            continue;
        }

        // Check if modifiers match exactly
        if (shortcut.ctrl_ == kCtrl && shortcut.shift_ == kShift &&
            shortcut.alt_ == kAlt) {
            // Only trigger on the exact frame the main key is pressed
            if (utils::IsKeyPressedWrapped(shortcut.key_)) {
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
