---
sidebar_position: 4
---

# Node Editing

How a node's values and settings are edited in the editor. The fields are
drawn **inside the node**, under its pins. Every node shows its current values;
the fields can be edited when the node is the only selected one.

## Data flow

```
module (renderer capability)          editor                          graph
GetNodeComponents(id, type, props) -> ComponentAdapter draws/updates
                                      on_change_ of a component
NotifyPropertyChange(id, changes)  -> pending_property_changes_
                                      ApplyPropertyChange(...)       -> Node::SetProperty
                                                                        Graph::SetPinType
```

1. Each frame `Application::RefreshNodeFields()` asks the module's
   `IRendererCapability` for the components of every node, built from the
   node's **current properties** (`Node::Properties()`). The fields never keep
   a value of their own: what is shown is always what is in the graph.
2. `ui::ComponentAdapter` draws them in `NodeView::ContentBounds()` (world
   space, right after the node, see `FrameCallbacks::draw_node_content_`) and
   handles the input of the selected node.
3. A component callback reports new values to the handler set with
   `IRendererCapability::SetPropertyChangeHandler`. Changes are a
   `PropertyMap`, applied together (a literal changes its type and its value in
   one step).
4. `ui::ApplyPropertyChange` (`src/editor/ui/property_change.cpp`) writes them
   to the graph.

## Fields

| Component | Interaction |
| --- | --- |
| `TextField` | Click to edit, type, `Entrée` applies, `Échap` cancels |
| `NumberField` | Same as a text field; accepts `,` as decimal separator |
| `Checkbox` | Click toggles |
| `Select` | Click opens the list, click an option |
| `Button` | Click |

- A text or number field is applied on `Entrée` or when clicking elsewhere,
  and **only when valid**. While the text is invalid, its explanation is shown
  in red under the field. Clicking elsewhere with an invalid text drops it and
  shows a warning ("Modification annulée : ...").
- `TextField::validate_` (optional, set by the module) returns the reason why a
  text is invalid. Number fields are checked by the adapter: number, integer,
  bounds.
- While a field has the keyboard, editor shortcuts (`Suppr`, `Ctrl+C`, ...)
  are ignored.

## C module

| Node | Fields |
| --- | --- |
| Variable | Nom (C++ identifier), Type |
| Literal | Valeur (depends on the type), Type |
| Operator | Opérateur: `+ - * / %` |
| Condition | Condition (checkbox) |
| Function | Nom (C++ identifier) |
| Loop and others | none |

- Types: `int`, `float`, `bool`, `char`, `string`, stored as text in property
  `kType` (5). A missing or unknown type means `int`.
- Literal value: `double` for int/float (`NumberField`), `bool` for bool
  (`Checkbox`), `std::string` for char (exactly one ASCII character) and string.
  Changing the type converts the value (4.7 -> 5, "true" -> true, ...).
- Names: not empty, letters/digits/`_`, not starting with a digit, not a C or
  C++ reserved word (`int`, `class`, ...), not `main`.

## Type change and connections

A Variable or a Literal outputs the type it holds:
`INodeListCapability::ResolveNodeConfiguration(type, properties)` gives the
pins for the new properties (default: `GetNodeConfiguration(type)`, pins do not
depend on properties). Pins are matched by position.

A connected pin **never** changes type. If a change would modify a connected
pin, it is **refused as a whole**: nothing changes in the graph, a warning
names the pin and the linked nodes, the pin turns red and the first blocking
link is selected (nodes are unselected) so `Suppr` removes it. Then the type
can be changed. `Graph::SetPinType` enforces the same rule
(`TypeMismatchException` on a connected pin).

## Generated C++

The fields only change the graph. The C++ generation is not wired to the new
graph yet (the exporter capability is still a draft): it is out of scope here.
Once an exporter reads the node properties, the edited values will be used.
