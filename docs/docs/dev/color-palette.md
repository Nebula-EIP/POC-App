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
| `palette::base`         | Colors shared by several sections               |
| `palette::canvas`       | Canvas background, grid, rectangle selection    |
| `palette::top_bar`      | Menu bar and its drop-down menus                |
| `palette::popup`        | Node picker                                     |
| `palette::node`         | Node cards: header, body, title, pin names      |
| `palette::field`        | Values and settings drawn inside the nodes      |
| `palette::link`         | Selected link, link being dragged               |
| `palette::notification` | Warnings at the bottom of the window            |
| `palette::type`         | Pins and links, by data type                    |
| `palette::accent`       | Line above the node header, by node category    |

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

## Pin types and node categories

Pins and links take the color of their data type, looked up by the name the
module gives the type. A node gets a line above its header in the color of its
category (its root folder: `Flow/Loops` uses `Flow`). Both are tables: adding a
type or a category is one line.

```cpp
inline constexpr Entry kColors[] = {
    {kFlowTypeName, kFlow}, {"int", kInt}, {"float", kFloat},
    {"bool", kBool}, {"char", kChar}, {"string", kString},
};

palette::TypeColor("int");             // type::kInt, type::kUnknown if absent
palette::CategoryAccent("Flow/Loops"); // accent::kFlow, transparent if absent
```

The pins of the execution flow type (`type::kFlowTypeName`, `"void"`) are drawn
as triangles, the other pins as half discs. A pin without a link is hollow.

The nodes get the type names, categories and links through
`ui::NodePaint`, given once by the application with
`NodeCanvas::SetPaint()`.

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
