---
id: intro
---

# Core graph library

The core part of the codebase contains the graph structure used by all of the parts of the Nebula project.

It is composed of two main classes:
- NodeBase
- Graph

The graph class is the master of all nodes links.

A node can edit it's own values/behaviors but should never be able to edit things which are related to it's connections with other nodes.

If you want to edit pins or links, you must do so via the graph class.

For now it also contains all of the nodes classes used to handle C code.
