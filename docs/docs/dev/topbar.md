---
sidebar_position: 2
---

# Editor Top Bar

The top bar is the menu bar at the top of the editor window. It gives access
to the main actions.

It is implemented in `src/editor/ui/top_bar.hpp` and
`src/editor/ui/top_bar.cpp`. The menus are built by the application in
`Application::BuildMenus()` (`src/editor/application.cpp`).

## Rules

- The bar is drawn in screen space, after the canvas: pan and zoom never move
  it. It has a fixed height and always spans the whole window width, including
  after a resize.
- Menus open on click and close on a click outside, on a second click on their
  title, or with Escape. While a menu is open, hovering another title switches
  to it.
- A mouse press on the bar, or while a menu is open, never reaches the canvas.
- Menu items never call the core directly. Each one holds a **command** (a
  `std::function<void()>`) provided by the application.
- An item without a command is an action that is not implemented yet: it is
  greyed out and does nothing.

## Current menus

| Menu      | Items                                                          |
| --------- | -------------------------------------------------------------- |
| Fichier   | Nouveau, Ouvrir..., Enregistrer (not implemented yet), Quitter |
| Affichage | Recentrer, Zoom 100 %                                          |

- **Recentrer** centers the view on the nodes (the middle of their bounding
  box), in the part of the window below the bar. The zoom is unchanged.
- **Zoom 100 %** sets the zoom back to 1, keeping the center of the view in
  place.

Both use `render::Camera::CenterOn()` and `render::Camera::SetZoom()`.

## Adding a menu or an item

Add it in `Application::BuildMenus()`. The builder methods can be chained:

```cpp
top_bar_.AddMenu("Aide")
    .AddAction("Documentation", [this] { OpenDocumentation(); })
    .AddSeparator()
    .AddAction("Raccourcis");  // no command yet: greyed out
```

## Frame loop

```cpp
// Input: the bar goes first.
const bool captured = top_bar_.Update(ui::TopBar::ReadInput());  // may run a command
renderer_.ProcessInput(camera_, captured);  // the canvas ignores the mouse if captured

// Drawing, in the draw_ui_ callback of the renderer (screen space).
top_bar_.Draw();
```

`Update()` returns `true` when the mouse belongs to the bar this frame: a menu
is open, the cursor is over the bar, or the current press started on the bar.
A drag that started on the canvas keeps going when it crosses the bar.

Escape no longer closes the window (`SetExitKey(KEY_NULL)` in the renderer): it
closes the menus. Use Fichier > Quitter instead.

## Testing

`TopBar::Update()` does not read raylib directly: it takes a `TopBarInput`, and
the constructor accepts a text measuring function. The bar is therefore tested
without a window in `tests/editor/top_bar_test.cpp`. The camera functions are
tested in `tests/render/camera_tests.cpp`.
