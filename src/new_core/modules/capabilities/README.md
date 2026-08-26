Capabilities are parts of modules.
This folder is destined for interfaces of the capabilities used by the main app.

Please do not implement modules here !!!!!
Only interfaces !!!

core:
  type_list_capability: Provides every type introduced by the module
  node_list_capability: list of all nodes and their functionality

optional:
  validation_capability: check the current graph for errors (will be added later on)
  exporter_capability: export graph into code
  importer_capability: import code into graph
  renderer_capability: how the nodes are rendered & are interacted with