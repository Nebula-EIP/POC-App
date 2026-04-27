---
id: intro
---

# Core graph library

The core part of the codebase contains the graph structure used by all of the parts of the Nebula project.

It is composed of two main classes:
- NodeBase
- Graph

The graph class is the master of all nodes links.

A node can edit it's own values/behaviors but should never be able to edit things from other nodes.

If you want to edit links or inputs/outputs types, you must do so via the graph class.
