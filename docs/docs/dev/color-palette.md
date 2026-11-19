---
sidebar_position: 5
---

# Color Palette

Every color the editor draws with is named in one file: `palette.yaml`, at the
root of the repository. No color is written as numbers in the code: the code
references the palette by name, so changing a color in the file changes it
everywhere it is used.

## Changing a color

Edit `palette.yaml` and restart the editor: the file is read at start-up from
the working directory, **no rebuild needed**.

```yaml
canvas:
  background: "#23252D"                           # hex code
  grid: "#2E3037"
  selection_fill: { color: base.accent, opacity: 0.12 }

popup:
  header: node.header                             # another color, by name
```

A color is written in one of these ways:

| Form                                      | Meaning                             |
| ----------------------------------------- | ----------------------------------- |
| `"#23252D"`                               | Hex code, opaque                    |
| `"#23252D80"`                             | Hex code with an alpha channel      |
| `chasm.blue`                              | Another color, as `section.name`    |
| `{ color: "#000000", opacity: 0.25 }`     | A color with an opacity from 0 to 1 |
| `{ color: base.accent, opacity: 0.12 }`   | Same, from another color            |

Hex codes need quotes: in YAML, `#` starts a comment.

A copy of `palette.yaml` is compiled into the editor. It is used when the file
is missing, and for every color the file leaves out or gets wrong. Mistakes do
not stop the editor: they are printed at start-up and the color keeps its
built-in value.

```
palette: 'canvas.grid' has no color (hex codes need quotes: "#RRGGBB")
palette: 'canvas.loading_text': unknown color 'chasm.pink', using the default
palette: 'canvas.background': circular reference: canvas.background -> canvas.grid -> canvas.background, using the default
```

## Referencing a color in the code

```cpp
#include "utils/palette.hpp"

utils::DrawRectangleRecWrapped(bar, palette::Get().top_bar_.background_);

const palette::Palette::Popup &colors = palette::Get().popup_;  // many uses
```

`palette::Get()` is the palette the editor draws with: `main()` loads
`palette.yaml` into it with `palette::Set(palette::Palette::Load(...))`. Before
that (and in the tests) it is the built-in palette, `Palette::Default()`.

Colors are read at run time: do not copy them into `constexpr` or global
constants, which would be set before the file is loaded.

| Section          | Used by                                         |
| ---------------- | ----------------------------------------------- |
| `chasm`          | The Chasm palette of the design (see below)     |
| `base`           | Colors shared by several sections               |
| `canvas`         | Canvas background, grid, rectangle selection    |
| `top_bar`        | Menu bar and its drop-down menus                |
| `popup`          | Node picker, drawn like a node card             |
| `node`           | Node cards: header, body, title, pin names      |
| `field`          | Values and settings drawn inside the nodes      |
| `link`           | Selected link, link being dragged               |
| `notification`   | Warnings at the bottom of the window            |
| `type`           | Custom types and types without a color          |
| `type_names`     | Pins and links, by data type                    |
| `accent`         | Line at the top of the nodes, by kind of node   |
| `accent_nodes`   | Accent of single nodes, by name                 |
| `accent_categories` | Accent of the nodes, by category             |

## Adding a color

- **A pin type or a node accent**: one line in `type_names`, `accent_nodes` or
  `accent_categories`. No code.
- **A new color for the code**: one line in `palette.yaml`, one member in its
  section struct in `src/utils/palette.hpp`, and one line in
  `Palette::Fields()` (`src/utils/palette.cpp`) giving its `section.name`.
  `Palette.ThePaletteFileIsValidAndComplete` fails if one of the three is
  missing.

## The Chasm palette

The design's palette. The other sections reuse these colors by name
(`int: chasm.blue`), so the pins, links and accents stay in the palette.

| Name             | Value     | Name           | Value     |
| ---------------- | --------- | -------------- | --------- |
| `chasm.silver`   | `#AEAEB4` | `chasm.red`    | `#A8474A` |
| `chasm.slate`    | `#4E4F56` | `chasm.violet` | `#7E7FCD` |
| `chasm.charcoal` | `#24252C` | `chasm.orange` | `#D19D82` |
| `chasm.abyss`    | `#16171D` | `chasm.blue`   | `#76A1E1` |
| `chasm.green`    | `#6EA17D` |                |           |

## Pin types

Pins and links take the color of their data type, looked up by the name the
module gives the type:

| Type                    | Color  |
| ----------------------- | ------ |
| `int`                   | blue   |
| `float`                 | violet |
| `string`                | green  |
| `bool`                  | red    |
| custom (user defined)   | orange |
| any other (`char`...)   | grey   |
| `void` (execution flow) | silver, drawn as a triangle |

```yaml
type:
  custom: chasm.orange
  other: chasm.slate
type_names:
  void: chasm.silver
  int: chasm.blue
```

```cpp
palette::TypeColor("int");           // type_names.int
palette::TypeColor("char");          // type.other (grey)
palette::TypeColor("Player", true);  // type.custom (orange)
```

No module declares custom types yet: `is_custom` is there for when one does.
A pin without a link is hollow.

## Node accents

The line at the top of a node (and the marker before its name in the node
picker) tells what kind of node it is:

| Color | Nodes                                   | Rule                         |
| ----- | --------------------------------------- | ---------------------------- |
| Red   | Entry points: Function, its input/output | category `Functions`        |
| White | Branches (Condition) and loops          | category `Flow`, `Flow/...`  |
| Blue  | Nodes that set a variable               | node `Variable`              |
| None  | Everything else                         |                              |

A node is first looked up by name (`accent_nodes`), then by the root folder
of its category (`accent_categories`):

```cpp
palette::NodeAccent("Variable", "Data");   // accent.variable (blue)
palette::NodeAccent("For", "Flow/Loops");  // accent.branch (white)
palette::NodeAccent("Literal", "Data");    // accent.none (no line)
```

The nodes get the type names, accents and links through `ui::NodePaint`,
given once by the application with `NodeCanvas::SetPaint()`.

## Design values

| Element         | Value                                |
| --------------- | ------------------------------------ |
| Background      | `#23252D`                            |
| Grid            | `#2E3037`                            |
| Top bar         | `#22232C`, 57 px high                |
| Top bar text    | `#909195`                            |
| Top bar shadow  | `#000000` at 25 %, offset 4, blur 4  |
| Node header     | `#1F2027`                            |
| Node body       | `#16171D`                            |

## Build

The palette is read with [yaml-cpp](https://github.com/jbeder/yaml-cpp)
(fetched by CPM). CMake compiles `palette.yaml` into the editor
(`src/utils/palette_default.cpp.in`) and re-runs when the file changes.

## Testing

`tests/utils/palette_test.cpp` checks `Hex()`, `WithOpacity()`, the lookups,
the design values, that `palette.yaml` is valid and complete, and how a file is
read: overrides, references, opacities, new table entries, and every mistake
(missing quotes, bad hex code, unknown or circular reference, unknown name,
invalid YAML, missing file).
