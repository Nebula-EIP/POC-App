# User Documentation — C++ Visual Editor

## Overview

The C++ Visual Editor is a node-based interface that lets you design and visualize C++ programs graphically. Instead of writing code line by line, you build logic by connecting nodes together in a graph, making relationships between variables and literals immediately visible.

---

## Interface

### Top Bar

The top bar is divided into three menus:

**File**
- **New** — Create a new empty project.
- **Save** — Save the current project to disk.
- **Load** — Open an existing project file.
- **Quit** — Exit the application.

**Add**
- **Variable** — Add a variable node to the graph.
- **Literal** — Add a literal node to the graph.

**Edit**
- **Duplicate Selection** — Create a copy of the currently selected node(s).
- **Delete Selection** — Remove the currently selected node(s) from the graph.

---

### Graph View

The main area of the editor displays your program as an interactive graph. Each element of your program is represented as a **node**, which can be connected to other nodes via **edges** to express relationships and data flow.

- **Nodes** represent program elements such as variables or literal values.
- **Connections** between nodes represent relationships or data dependencies.
- Click and drag from a node's connector point to another node to link them together.

---

## Keyboard & Mouse Shortcuts

> **Note:** All keyboard shortcuts use a **QWERTY** layout.

| Input | Action |
|---|---|
| `H` | Toggle cursor visibility (hide / show) |
| `Q` | Add a new **Variable** node at the current position |
| `W` | Add a new **Literal** node at the current position |
| `Left Click` + `D` | Duplicate the selected node |
| `Scroll Wheel` | Zoom in / Zoom out in the graph view |
| `Middle Click` (scroll wheel button) | Pan / navigate the graph view |
| `Space` + `Left Click` | Pan / navigate the graph view |

---

## Quick Start

1. Launch the editor.
2. Use **File → New** or press nothing — a blank graph is ready by default.
3. Press `Q` to place a variable node, or `W` to place a literal node.
4. Click and drag from one node's output to another node's input to connect them.
5. Use `Scroll Wheel` to zoom in and out, and `Space + Left Click` to navigate around the graph.
6. When you're done, save your work via **File → Save**.

---

## Tips

- Use `H` to hide the cursor if you want a cleaner view of your graph during a presentation or review.
- You can duplicate a node quickly with `Left Click + D` instead of going through the **Edit** menu — useful when building repetitive structures.
- Both `Middle Click` and `Space + Left Click` pan the view; use whichever feels more comfortable for your mouse setup.