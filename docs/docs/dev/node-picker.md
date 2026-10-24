---
sidebar_position: 4
---

# Adding Nodes (Node Picker)

A new project starts with an empty canvas. Nodes are added one at a time from
the **Node picker**, a searchable tree of every node provided by the loaded
modules.

It is implemented in `src/editor/ui/node_picker.hpp` and
`src/editor/ui/node_picker.cpp`, and wired in `Application::BuildMenus()` and
`Application::ProcessInput()` (`src/editor/application.cpp`).

## Opening it

| From | Where the picker opens | Where the node appears |
| --- | --- | --- |
| Top bar: **Node > Add Node** | Under the **Node** title | Centre of the view (shifted when a node is already there) |
| Shortcut **Shift+A** (`node.add_menu` action) | At the mouse cursor | Canvas position under the cursor when the picker was opened |

Both open the same list. A link released on empty canvas opens it too, in
connection mode (see below).

## Using it

Without a search the picker shows a **tree**, like Unity's Shader Graph:

- one folder per module, then the module's category folders. A node with the
  category `"Flow/Loops"` sits in `C > Flow > Loops`; a node without category
  sits directly in its module folder. Sub-folders come before nodes, in the
  order of first appearance.
- Folders start closed; when only one module is loaded its folder starts
  open. A click or **Enter** toggles a folder, **Right** opens it (or enters
  it when open), **Left** closes it (or goes up to the parent folder).

Typing a **search** replaces the tree with a **flat list** of the matching
nodes, without folders: case-insensitive substring of the node name (folder
names are not searched), names starting with the text first. Closed folders
do not hide results. Clearing the search brings the tree back as it was.

In both modes:

- **Up/Down** move the selection, the mouse wheel scrolls, moving the mouse
  over a row selects it. In the flat list the first result is selected, so
  typing then **Enter** creates it.
- **Enter** or a click on a node creates it. Escape or a click outside closes
  the picker without creating anything.
- Each pick creates exactly one node and closes the picker. The created node
  becomes the selection, so it can be moved right away.
- The box keeps the same size while folders open or a search is typed.

While the picker is open it owns the mouse and the keyboard: shortcuts
(Delete, Ctrl+C, Shift+A, ...) are ignored so that typing a search never edits
the graph.

The character of the shortcut that opened the picker is never typed into the
search: it can arrive one frame after the key press (Windows) and repeats while
the key is held. The picker ignores typed characters on its first frame and
until the shortcut key (`ActionManager::IsShortcutKeyDown("node.add_menu")`,
AZERTY included) is released; after that, every key types normally.

## From a dropped link (connection mode)

Dragging a link from a pin and releasing it on **empty canvas** opens the
picker at the cursor, like Unity's Shader Graph. The ghost link stays drawn
from the pin to the drop point while the picker is open.

- It lists only the pins that can be linked to the dragged one, **one entry
  per pin**, labelled `Node » pin (type)` (e.g. `Operator » left (int)` and
  `Operator » right (int)`). Dragged from an output: inputs of the same type;
  from an input: outputs of the same type. Only the nodes of the dragged
  node's module are offered, since type ids are given per module.
- Same tree (module > categories, no empty folder) and keys as the normal
  picker. The search matches the **node name and the pin name**
  (case-insensitive), not the type. Nothing compatible: `Aucun node`.
- A pick creates the node with the module's defaults, placed so that the
  picked pin sits on the drop point, links it to the dragged pin with
  `ui::RequestConnection` (same rules and messages as a manual link; a link
  on a connected input replaces it) and selects it. If the link is refused,
  the node is removed again and the error is shown.
- Escape or a click outside cancels: no node, no link, the ghost disappears.
- Released on a pin, the link is made directly. Released on a node body, on
  the top bar or outside the window, nothing happens.

Shift+A and **Node > Add Node** keep listing every node.

How it is built:

- `NodeCanvas::Update(CanvasInput)` (raylib-free; `ProcessInput` reads raylib
  into a `CanvasInput`) records the drop, read once with
  `NodeCanvas::PopPendingLinkDrop()`. A drag released while another part of
  the editor owns the pointer (top bar) ends there without anything.
- `ui::CheckConnectionRule()` (`connection_request.hpp`) holds the rules
  (pins, opposite directions, different nodes, same type). Both
  `RequestConnection()` and the candidate list use it, so the picker never
  offers a link that would be refused.
- `src/editor/ui/link_drop.{hpp,cpp}`: `FindDraggedPin()`,
  `LinkDropEntries()` (catalog to entries), `ResolvePickerPin()` and
  `LinkNewNode()`. Catalog pins (`NodePinDescriptor`) have no id: an entry
  keeps the pin's direction and **index among the pins of that direction**,
  which is the order `Graph::AddInputPin`/`AddOutputPin` follow when the node
  is created; `ResolvePickerPin()` reads the real id back from the created
  node and checks the name and type.
- `NodePicker::OpenForConnection()` shows the given entries and calls back
  with the picked `NodePickerEntry` (its `pin_` included); the entries of
  `SetEntries()` come back at the next `Open()`.
- `Application::HandleLinkDrop()` / `CreateLinkedNode()` wire it; the ghost
  link is `Application::link_drop_`, cleared on pick or when the picker
  closes.

## Categories

A module gives the folder of each node with `NodeMetadata::category_`
(folders separated by `/`). With the generic `capa::NodeListCapability`, pass
it as the last argument of `RegisterNode(name, description, config,
category)`; it defaults to empty. The catalog exposes it as
`NodeDescriptor::category_` (and the description as
`NodeDescriptor::description_`).

The C module uses: `Data` (Variable, Literal), `Math` (Operator), `Flow`
(Condition), `Flow/Loops` (Loop, For), `Functions` (Function, FunctionInput,
FunctionOutput), `IO` (Print).

## Where the list comes from

`Application::LoadModule()` refreshes the `core::NodeCatalog`, which assigns
the node type ids, then gives the picker its entries with
`ui::EntriesFromCatalog()`. No node is created at startup.

When a node is picked, `Application::CreateNode()` looks the type up in the
catalog, asks **the module that provides it** for a fresh
`NodeConfiguration` (`INodeListCapability::GetNodeConfiguration`), and builds
the node from it: pins and the module's default property values. Each node
gets its own copy of the defaults, so several instances of the same node never
share values.

## Testing

`tests/editor/node_picker_test.cpp` drives the picker through
`NodePickerInput` with a fixed-width text measurer, without a window: tree
building, folders opening and closing (mouse and keyboard), flat search,
picking, closing, scrolling, the conversion from the catalog of the C
module, and the connection mode.

`tests/editor/link_drop_test.cpp` checks the candidates against the C module
(direction, type, module, one entry per pin), the pin of the created node, and
the link made after a pick (refused link removes the node).
`tests/editor/node_canvas_test.cpp` checks where a link drag ends: empty
canvas, pin, node body, outside the window, over the top bar.
