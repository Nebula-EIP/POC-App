---
id: node
---

# NodeBase class

The NodeBase class is a data class that stores informations about itself (id, kind, ...) and it's connections with other nodes.

It can only be created via the core::Graph::AddNode method and is thus owned by said Graph.

As is, you can't edit any of it's metadatas from a pointer nor it's links, to do so you must use the Graph's class methods. 

## Available informations

```cpp
int main()
{
    core::Graph graph;
    core::NodeBase *node = graph.AddNode(core::NodeBase::NodeKind::kLiteral)

    uint32_t id = node->id();
    
    core::NodeBase::NodeKind kind = node->kind();

    uint8_t in_pin_count = node->GetInputPinCount();
    uint8_t out_pin_count = node->GetOutputPinCount();

    // You can get a pin id when adding one via core::Graph::AddxxxPin()
    bool in_pin_exists = node->InputPinExists(1);
    bool out_pin_exists = node->InputOutExists(1);

    bool in_pin_connected = node->IsInputPinConnected(1);
    bool out_pin_connected = node->IsOutputPinConnected(1);

    core::NodeBase::PinDataType in_pin_type = node->GetInputPinType(1);
    core::NodeBase::PinDataType out_pin_type = node->GetOutputPinType(1);

    std::string in_pin_name = node->GetInputPinName(1);
    std::string out_pin_name = node->GetOutputPinName(1);
}
```
