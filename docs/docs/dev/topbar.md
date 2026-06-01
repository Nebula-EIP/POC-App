---
sidebar_position: 2
---

# Editor Top Bar

The top bar is the horizontal banner displayed at the top of the editor. It provides the main application menu, with dropdowns and actions associated with each entry.

It is implemented in `src/editor/ui/top_bar.hpp` and `src/editor/ui/top_bar.cpp`.

## Purpose

The top bar is used to:

- display the editor's main menus
- open and close dropdown lists
- trigger actions via `on_click` callbacks
- temporarily block graph interactions while a menu is open

## Data Structure

The public API relies on three structures.

### `MenuItem`

Represents a clickable entry in a menu.

```cpp
struct MenuItem {
    std::string label;
    std::function<void()> on_click;
};
```

- `label`: text shown in the dropdown
- `on_click`: callback executed when the entry is clicked

### `Menu`

Represents a menu in the top bar.

```cpp
struct Menu {
    std::string label;
    std::vector<MenuItem> items;
};
```

- `label`: label visible on the bar
- `items`: the set of actions available in the menu

### `TopBarStyle`

Describes the appearance of the top bar.

Main fields include:

- `bar_height`: height of the main banner
- `item_height`: height of each dropdown line
- `horizontal_padding`: horizontal padding in tabs
- `item_padding`: horizontal padding in items
- `minimum_menu_width`: minimum width of a menu
- `font_size`: text size
- background, hover, text, and border colors

## Behavior

The `TopBar` class manages the interactive state of the banner.

### Construction

```cpp
TopBar(std::vector<Menu> menus, TopBarStyle style = {});
```

The constructor receives the full list of menus and an optional style.

### Update

```cpp
void Update();
```

`Update()` reads the mouse position and left-click state to:

- detect the hovered menu
- detect the hovered item in an open menu
- open or close a menu
- call the clicked item's callback

### Drawing

```cpp
void Draw() const;
```

`Draw()` renders:

- the main bar
- the menu tabs
- the active dropdown menu
- visual hover states for the active menu or item

### Blocking Graph Input

```cpp
bool BlocksGraphInput() const noexcept;
```

This method indicates whether the top bar should prevent interactions with the graph.

It returns `true` when a menu is open or when a top bar element is under the mouse.

## Placement Rules

- each menu occupies a width calculated from the text and margins
- the minimum width is controlled by `TopBarStyle::minimum_menu_width`
- the dropdown uses the same width as its tab
- if the menu would be drawn outside the window, it is shifted left to remain visible

## Usage Example

```cpp
using editor_ui::Menu;
using editor_ui::MenuItem;
using editor_ui::TopBar;

TopBar top_bar({
    {"File", {
        {"New", [] { /* create a new project */ }},
        {"Open", [] { /* open a project */ }},
    }},
    {"Edit", {
        {"Undo", [] { /* undo the last action */ }},
    }},
});

// In the main loop
top_bar.Update();
top_bar.Draw();

if (top_bar.BlocksGraphInput()) {
    // Disable graph interactions while the menu is open
}
```

## Important Notes

- `SetMenus()` replaces the entire menu list and closes the current menu
- `CloseMenu()` resets the hovered item state
- `OpenMenu()` checks bounds before opening a menu
- `ActivateItem()` silently ignores invalid indices

## Current Limitations

- the top bar depends on `raylib` for mouse input and rendering
- the most robust unit tests therefore focus on the public API and observable internal state
- full interaction tests would require an additional abstraction layer to simulate user input
