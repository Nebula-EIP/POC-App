---
sidebar_position: 3
---

# Canvas Editing

How nodes and connections are edited on the canvas. The actions live in
`Application` (`src/editor/application.cpp`), on top of the selection and
movement handled by `NodeCanvas` (`src/editor/ui/node_canvas.cpp`).

## Actions

| Action | Mouse / keyboard | Menu |
| --- | --- | --- |
| Connect two pins | Drag from a pin to another pin | - |
| Select a connection | Click on it (it turns white) | - |
| Select nodes | Click, Ctrl+click, or drag a rectangle | - |
| Delete the selection | `Suppr` | Édition > Supprimer |
| Duplicate the selected nodes | `Ctrl+D` | Édition > Dupliquer |

- **Delete** removes the selected connection and the selected nodes.
  `Graph::RemoveNode` also removes every connection of a deleted node.
- **Duplicate** calls `Graph::DuplicateNode`: same type, same pins (ids, names,
  types) and same properties, a new id and **no connection**. Copies appear
  30 units down-right of their original and replace it in the selection.

## Connections

A pin drag is turned into a connection by `ui::RequestConnection`
(`src/editor/ui/connection_request.hpp`). The drag works in both directions
(output to input or input to output). When the connection is refused, the pin
it was dropped on turns red and a warning is shown at the bottom center of the
window:

| Case | Message |
| --- | --- |
| Two inputs / two outputs | Impossible de relier deux entrées (sorties)... |
| Same node on both ends | Un node ne peut pas être relié à lui-même. |
| Different types | Types incompatibles : la sortie « int » ne peut pas aller dans l'entrée « bool ». |
| Any other graph error | Connexion impossible : ... |

Releasing on the pin the drag started from does nothing.

## Warnings

Warnings go through `ui::NotificationStack`
(`src/editor/ui/notification_stack.hpp`), fed by `Application::ShowError()`.
Each warning stays 4 seconds and fades out during its last half second. A new
warning never replaces the others: it appears at the bottom and pushes the
older ones up. At most 5 are shown; beyond that the oldest one is dropped.

Input and output pins are numbered separately, so the first input and the
first output of a node share the same id. Always pass the pin side to
`NodeView::GetPinPosition(pin_id, is_input)`, or links are drawn from the
wrong pin.
