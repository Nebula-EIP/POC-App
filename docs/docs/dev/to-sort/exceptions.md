---
id: exceptions
---

# Exceptions

When you catch exceptions from one of the project's function, they are a couple of ways you can retreive the error msg.

```cpp
#include "graph.hpp"

int main() {
    LiteralNode *node;
            
    try {
        node = graph.AddNode<LiteralNode>(core::NodeBase::NodeKind::kLiteral);
    } catch (core::InvalidNodeKindException &e) {
        // You can use e.what() to get the error msg
        std::cerr << "Error: " << e.what() << std::endl;
        // You can use e.GetDetailedMessage() to a complete error log
        // Format: "Error: <message>\n  at <function> (<file>:<line>:<column>)"
        std::cerr << e.GetDetailedMessage() << std::endl;
        // Same as before but with std::format support
        // Format: "Error: {fmt+args} at <function> (<file>:<line>:<column>)"
        std::cerr << e.GetFormattedMessage("Error #{}: ", 42);
    }
}
```