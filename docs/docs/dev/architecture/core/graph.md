---
id: graph
---

# Graph class

## What is stored in the graph ?

The graph holds control over nodes, it is a tool used to handle their lifetime & links.

The graph are made to represent a function, as such, it holds metadatas:
- Project name `std::string` - `core::Graph::GetProjectName()`
- Version `std::string` - `core::Graph::GetVersion()`
- Author `std::string` - `core::Graph::GetAuthor()`
- Creation date `std::chrono::system_clock::time_point` - `core::Graph::GetCreatedAt()`
- Last edition date `std::chrono::system_clock::time_point` - `core::Graph::GetModifiedAt()`

## How to use it ?

Here is examples for all methods accessible from the core::Graph class.

### Create/Load a graph

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;

    // You can set metadatas on the current graph, remember they represent functions
    graph.SetProjectName("Hello world !");
    graph.SetAuthor("Alpha Diallo");

    // You can load a graph from a savefile (following the .nebula JSON format)
    graph = core::Graph::LoadFromFile("/tmp/savefile.nebula");
}
```

### Save a graph

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;

    // You can save the graph as JSON format
    graph.UpdateModifiedTime();
    auto res = graph.SaveToFile("/tmp/savefile"); // The file will be saved under /tmp/savefile.nebula
    if (res) {
        std::cout << "Files saved sucessfully" << std::endl;
    } else {
        std::cerr << "Error while saving the file: " << res.error() << std::endl;
    }
}
```

### Add a node

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;
    core::NodeBase *base_node;
    core::LiteralNode *literal_node;

    try {
        // You can either get a NodeBase class back or the specific node class you created.
        // NodeBase allows you to execute all methods from the core::Graph class.
        // The node class of your specific node allows you to edit some node specific stuff.

        // NodeBase
        base_node = graph.AddNode(core::NodeBase::NodeKind::kLiteral);

        // Specific type, the type must match the kind
        literal_node = graph.AddNode<core::LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    } catch (core::InvalidNodeKindException &e) {
        // get thrown when the NodeKind argument is equal to kUndefined
        std::cerr << e.GetDetailedMessage() << std::endl;
    }
}
```

### Delete a node

Deleting a node automatically disconnect all nodes connected to it.

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;
    core::NodeBase *node = graph.AddNode(core::NodeBase::NodeKind::kLiteral);

    // You can remove a node either pointer
    try {
        graph.RemoveNode(node);
    } catch (core::NodeNotFoundException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    }
    // You can also do it via the node's id
    try {
        graph.RemoveNode(node.id());
    } catch (core::NodeNotFoundException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    }
}
```

### Retrieve nodes

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;
    core::NodeBase *node1 = graph.AddNode(core::NodeBase::NodeKind::kVariable);
    core::NodeBase *node2 = graph.AddNode(core::NodeBase::NodeKind::kVariable);
    core::NodeBase *node3 = graph.AddNode(core::NodeBase::NodeKind::kVariable);

    // You can get all nodes at once
    const std::vector<std::unique_ptr<NodeBase>> &nodes = graph.GetAllNodes();

    // You can fetch a specific node with it's id or pointer.
    // There is a regular and a templated Graph::GetNode method
    // choose the correct one depending on what you need to access
    // in the node. (Templated more costly)
    
    // Regular:
    core::NodeBase *a = graph.GetNode(node.id());
    core::NodeBase *b = graph.GetNode(node);

    // Templated:
    core::VariableNode *c = graph.GetNode<VariableNode>(node.id());
    core::VariableNode *c = graph.GetNode<VariableNode>(node);
    // If you pick the wrong type to cast to, expect undefined behaviors
}
```

### Add pins

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;
    core::NodeBase *node = graph.AddNode(core::NodeBase::NodeKind::kVariable);

    graph.AddInputPin(node, "Size in m", core::NodeBase::NodeKind::kFloat);
    graph.AddInputPin(node, "Weight in kg", core::NodeBase::NodeKind::kFloat);
    graph.AddInputPin(node, "IMC", core::NodeBase::NodeKind::kFloat);
}
```

### Remove pins

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;
    core::NodeBase *node = graph.AddNode(core::NodeBase::NodeKind::kVariable);

    uint8_t pin1 = graph.AddInputPin(node, "Size in m", core::NodeBase::NodeKind::kFloat);
    uint8_t pin2 = graph.AddInputPin(node, "Weight in kg", core::NodeBase::NodeKind::kFloat);
    uint8_t pin3 = graph.AddOutputPin(node, "IMC", core::NodeBase::NodeKind::kFloat);

    graph.RemoveInputPin(node, pin1);
    graph.RemoveInputPin(node, pin2);
    graph.RemoveOutputPin(node, pin3);
}
```

### Link nodes

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;
    core::NodeBase *from = graph.AddNode(core::NodeBase::NodeKind::kVariable);
    core::NodeBase *to = graph.AddNode(core::NodeBase::NodeKind::kVariable);

    try {
        // The two zeros are the ids of the nodes pins
        graph.Link(from, 0, to, 0);
    } catch (core::NodeNotFoundException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    } catch (core::InvalidPinIndexException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    } catch (core::IncompatiblePinTypesException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    } catch (core::CircularDependencyException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    }
}
```

### Unlink nodes

```cpp
#include "graph.hpp"

int main() {
    core::Graph graph;
    core::NodeBase *from = graph.AddNode(core::NodeBase::NodeKind::kVariable);
    core::NodeBase *to = graph.AddNode(core::NodeBase::NodeKind::kVariable);

    graph.Link(from, 0, to, 0);

    try {
        // The two zeros are the ids of the nodes pins
        graph.Unlink(from, 0, to, 0);
    } catch (core::NodeNotFoundException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    } catch (core::InvalidPinIndexException &e) {
        std::cerr << e.GetDetailedMessage() << std::endl;
    }
}
```
