/**
 * @file action_manager.hpp
 * @brief ActionManager class for managing shortcuts and actions.
 *
 * @author Created by mathys-f
 * @date Created on 06-10-2026
 */

#pragma once

#include <unordered_map>

#include "action.hpp"

namespace editor::actions {

/**
 * @brief Manages actions and their keyboard shortcuts.
 */
class ActionManager {
   public:
    ActionManager() = default;
    ~ActionManager() = default;

    /**
     * @brief Registers an action in the system.
     * @param action The action to register.
     */
    void RegisterAction(Action action);

    /**
     * @brief Triggers an action by its ID.
     * @param id The ID of the action to trigger.
     */
    void TriggerAction(const std::string &id) const;

    /**
     * @brief Processes all registered shortcuts based on current keyboard
     * state. Should be called each frame.
     */
    void ProcessShortcuts() const;

    /**
     * @brief Gets an action by its ID.
     * @return Pointer to the action, or nullptr if not found.
     */
    const Action *GetAction(const std::string &id) const;

   private:
    std::unordered_map<std::string, Action> actions_;
};

}  // namespace editor::actions
