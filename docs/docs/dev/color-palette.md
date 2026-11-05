---
sidebar_position: 5
---

# Color Palette

Every color the editor draws with is named in one file:
`src/utils/palette.hpp`. No color is written as numbers anywhere else: the
code references the palette by name, so changing a color in the palette
changes it everywhere it is used.

## Referencing a color

Colors are grouped by the part of the editor they belong to:

```cpp
#include "utils/palette.hpp"

ClearBackground(palette::canvas::kBackground);
utils::DrawRectangleRecWrapped(bar, palette::top_bar::kBackground);
```

| Section                 | Used by                                         |
| ----------------------- | ----------------------------------------------- |
| `palette::chasm`        | The Chasm palette of the design (see below)     |
| `palette::base`         | Colors shared by several sections               |
| `palette::canvas`       | Canvas background, grid, rectangle selection    |
| `palette::top_bar`      | Menu bar and its drop-down menus                |
| `palette::popup`        | Node picker, drawn like a node card             |
| `palette::node`         | Node cards: header, body, title, pin names      |
| `palette::field`        | Values and settings drawn inside the nodes      |
| `palette::link`         | Selected link, link being dragged               |
| `palette::notification` | Warnings at the bottom of the window            |
| `palette::type`         | Pins and links, by data type                    |
| `palette::accent`       | Line at the top of the nodes, by kind of node   |

## Adding a color

Add one line in the right section, from the hex code of the design and an
optional opacity (from 0 to 1):

```cpp
namespace canvas {
inline constexpr utils::WrappedColor kBackground = Hex(0x23252D);
inline constexpr utils::WrappedColor kGrid = Hex(0x2E3037);
inline constexpr utils::WrappedColor kMyColor = Hex(0x76A0E0);       // new
inline constexpr utils::WrappedColor kMyShadow = Hex(0x000000, 0.25F);  // new
}  // namespace canvas
```

A color can reuse another one (`kDropdown = top_bar::kBackground`) or another
one with a different opacity (`WithOpacity(base::kAccent, 0.12F)`). Colors are
`constexpr`: they cost nothing at run time.

## The Chasm palette

The design's palette. The other sections reuse these colors by name
(`kInt = chasm::kBlue`), so the pins, links and accents stay in the palette.

| Name             | Value     | Name             | Value     |
| ---------------- | --------- | ---------------- | --------- |
| `chasm::kSilver`   | `#AEAEB4` | `chasm::kRed`    | `#A8474A` |
| `chasm::kSlate`    | `#4E4F56` | `chasm::kViolet` | `#7E7FCD` |
| `chasm::kCharcoal` | `#24252C` | `chasm::kOrange` | `#D19D82` |
| `chasm::kAbyss`    | `#16171D` | `chasm::kBlue`   | `#76A1E1` |
| `chasm::kGreen`    | `#6EA17D` |                  |           |

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

```cpp
inline constexpr Entry kColors[] = {
    {kFlowTypeName, kFlow}, {"int", kInt}, {"float", kFloat},
    {"string", kString}, {"bool", kBool},
};

palette::TypeColor("int");           // type::kInt
palette::TypeColor("char");          // type::kOther (grey)
palette::TypeColor("Player", true);  // type::kCustom (orange)
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

A node is first looked up by name (`accent::kNodes`), then by the root folder
of its category (`accent::kCategories`):

```cpp
palette::NodeAccent("Variable", "Data");   // accent::kVariable (blue)
palette::NodeAccent("For", "Flow/Loops");  // accent::kBranch (white)
palette::NodeAccent("Literal", "Data");    // accent::kNone (no line)
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

## Testing

`tests/utils/palette_test.cpp` checks `Hex()`, `WithOpacity()`, the lookups
and the design values.
