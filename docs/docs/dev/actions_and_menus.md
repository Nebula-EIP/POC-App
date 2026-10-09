# Action and Shortcut System

Nebula includes a modular action system that allows developers to register commands, assign keyboard shortcuts to them, and expose them identically via contextual menus and toolbars.

## Adding a new Action

To add a new action, simply register it via the `ActionManager` in `Application::BuildMenus` (or your startup logic).

### 1. Register the Action

```cpp
// 1. Define the action ID, Display Name, Shortcut, and Callback
action_manager_.RegisterAction({
    "node.delete",                 // ID
    "Delete Node",           // Display Name
    {utils::WrappedKey::kDelete, false, false, false}, // Shortcut (Key, Ctrl, Shift, Alt)
    [this] { DeleteSelectedNode(); } // Callback
});
```

### 2. Add it to a Menu (TopBar or Context Menu)

Because the action is now registered, you can retrieve it by its ID and use it in menus, guaranteeing that the name, shortcut text, and behavior stay synchronized.

**In the Top Bar:**
```cpp
const auto* delete_action = action_manager_.GetAction("node.delete");

top_bar_.AddMenu("Édition")
    .AddAction(delete_action->display_name_, 
               delete_action->callback_, 
               delete_action->shortcut_.ToString());
```

**In a Context Menu:**
```cpp
const auto* delete_action = action_manager_.GetAction("node.delete");

context_menu_.AddAction(delete_action->display_name_, 
                        delete_action->callback_, 
                        delete_action->shortcut_.ToString());
```

### 3. How Shortcuts are processed

Every frame, `Application::ProcessInput` calls `action_manager_.ProcessShortcuts()`. This iterates through all registered actions and checks the keyboard state. 
If an action's shortcut is pressed, its callback is executed automatically! You do not need to add explicit `if (IsKeyPressed(...))` statements in your input loops anymore.
