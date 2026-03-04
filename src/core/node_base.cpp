#include "node_base.hpp"

#include <algorithm>

#include "nodes/literal_node.hpp"

core::NodeBase::Connection::Connection(NodeBase *n, uint8_t p)
    : node(n), pin(p) {}

bool core::NodeBase::Connection::IsConnected() const { return node != nullptr; }

core::NodeBase::NodeBase(uint32_t id, NodeKind kind) : id_(id), kind_(kind) {}

core::NodeBase::~NodeBase() {
    // Clean up parent connections
    for (auto* conn : parents_) {
        delete conn;
    }
    parents_.clear();

    // Clean up children connections
    for (auto& child_vec : childrens_) {
        for (auto* conn : child_vec) {
            delete conn;
        }
        child_vec.clear();
    }
    childrens_.clear();
}

uint32_t core::NodeBase::id() const { return id_; }

core::NodeBase::NodeKind core::NodeBase::kind() const { return kind_; }

// parents_ vector already filled by the Graph class
core::NodeBase::Connection *core::NodeBase::parent(uint8_t in_pin) const {
    return parents_[in_pin];
}

// childrens_ vector already filled by the Graph class
const std::vector<core::NodeBase::Connection *> &core::NodeBase::childrens(
    uint8_t out_pin) const {
    return childrens_[out_pin];
}

void core::NodeBase::SetParent(uint8_t in_pin, NodeBase *parent,
                               uint8_t parent_pin) {
    parents_[in_pin]->node = parent;
    parents_[in_pin]->pin = parent_pin;
    parents_[in_pin]->type = parent->GetOutputPinType(parent_pin);
}

void core::NodeBase::AddChild(uint8_t out_pin, NodeBase *child,
                              uint8_t child_pin) {
    auto* conn = new Connection(child, child_pin);
    conn->type = GetOutputPinType(out_pin);
    childrens_[out_pin].push_back(conn);
}

void core::NodeBase::ClearParent(uint8_t pin) {
    parents_[pin]->node = nullptr;
    parents_[pin]->pin = 0;
    parents_[pin]->type = PinDataType::kUndefined;
}

void core::NodeBase::RemoveChild(uint8_t out_pin, NodeBase *node,
                                 uint8_t in_pin) {
    auto &children = childrens_[out_pin];
    auto it = std::find_if(children.begin(), children.end(),
                           [node, in_pin](Connection* conn) {
                               return conn && conn->node == node && conn->pin == in_pin;
                           });
    
    if (it != children.end()) {
        delete *it;
        children.erase(it);
    }
}

void core::NodeBase::InitializeConnections() {
    // Initialize parent connections (one per input pin)
    parents_.clear();
    parents_.reserve(GetInputPinCount());
    for (uint8_t i = 0; i < GetInputPinCount(); ++i) {
        parents_.push_back(new Connection());
    }

    // Initialize children connections (one vector per output pin)
    childrens_.clear();
    childrens_.reserve(GetOutputPinCount());
    for (uint8_t i = 0; i < GetOutputPinCount(); ++i) {
        childrens_.push_back(std::vector<Connection *>());
    }
}

// Helper functions for enum to/from string conversion

std::string core::NodeKindToString(core::NodeBase::NodeKind kind) {
    switch (kind) {
        case core::NodeBase::NodeKind::kLiteral:
            return "Literal";
        case core::NodeBase::NodeKind::kVariable:
            return "Variable";
        case core::NodeBase::NodeKind::kFunction:
            return "Function";
        case core::NodeBase::NodeKind::kFunctionInput:
            return "FunctionInput";
        case core::NodeBase::NodeKind::kFunctionOutput:
            return "FunctionOutput";
        case core::NodeBase::NodeKind::kOperator:
            return "Operator";
        case core::NodeBase::NodeKind::kCondition:
            return "Condition";
        case core::NodeBase::NodeKind::kLoop:
            return "Loop";
        case core::NodeBase::NodeKind::kUndefined:
        default:
            return "Undefined";
    }
}

core::NodeBase::NodeKind core::StringToNodeKind(const std::string& str) {
    if (str == "Literal") return core::NodeBase::NodeKind::kLiteral;
    if (str == "Variable") return core::NodeBase::NodeKind::kVariable;
    if (str == "Function") return core::NodeBase::NodeKind::kFunction;
    if (str == "FunctionInput") return core::NodeBase::NodeKind::kFunctionInput;
    if (str == "FunctionOutput") return core::NodeBase::NodeKind::kFunctionOutput;
    if (str == "Operator") return core::NodeBase::NodeKind::kOperator;
    if (str == "Condition") return core::NodeBase::NodeKind::kCondition;
    if (str == "Loop") return core::NodeBase::NodeKind::kLoop;
    return core::NodeBase::NodeKind::kUndefined;
}

std::string core::PinDataTypeToString(core::NodeBase::PinDataType type) {
    switch (type) {
        case core::NodeBase::PinDataType::kInt:
            return "Int";
        case core::NodeBase::PinDataType::kFloat:
            return "Float";
        case core::NodeBase::PinDataType::kBool:
            return "Bool";
        case core::NodeBase::PinDataType::kString:
            return "String";
        case core::NodeBase::PinDataType::kVoid:
            return "Void";
        case core::NodeBase::PinDataType::kUndefined:
        default:
            return "Undefined";
    }
}

core::NodeBase::PinDataType core::StringToPinDataType(const std::string& str) {
    if (str == "Int") return core::NodeBase::PinDataType::kInt;
    if (str == "Float") return core::NodeBase::PinDataType::kFloat;
    if (str == "Bool") return core::NodeBase::PinDataType::kBool;
    if (str == "String") return core::NodeBase::PinDataType::kString;
    if (str == "Void") return core::NodeBase::PinDataType::kVoid;
    return core::NodeBase::PinDataType::kUndefined;
}

std::expected<std::unique_ptr<core::NodeBase>, std::string> core::NodeBase::Deserialize(
    const nlohmann::json& json, Graph* /*graph*/) {
    // Validate required fields
    if (!json.contains("id") || !json.contains("kind")) {
        return std::unexpected("Missing required fields: id or kind");
    }

    uint32_t id;
    std::string kind_str;
    
    try {
        id = json["id"].get<uint32_t>();
        kind_str = json["kind"].get<std::string>();
    } catch (const std::exception& e) {
        return std::unexpected(std::string("Failed to parse node fields: ") + e.what());
    }

    NodeKind kind = StringToNodeKind(kind_str);
    if (kind == NodeKind::kUndefined) {
        return std::unexpected(std::string("Unknown node kind: ") + kind_str);
    }

    // Dispatch to appropriate node type deserializer
    switch (kind) {
        case NodeKind::kLiteral: {
            auto node = LiteralNode::DeserializeHelper(json, id);
            if (!node) {
                return std::unexpected("Failed to deserialize LiteralNode");
            }
            // Initialize connections after construction
            node->InitializeConnections();
            return std::unique_ptr<NodeBase>(node.release());
        }

        case NodeKind::kVariable:
        case NodeKind::kFunction:
        case NodeKind::kFunctionInput:
        case NodeKind::kFunctionOutput:
        case NodeKind::kOperator:
        case NodeKind::kCondition:
        case NodeKind::kLoop:
            return std::unexpected(std::string("Node kind not yet implemented: ") + kind_str);

        case NodeKind::kUndefined:
        default:
            return std::unexpected("Cannot deserialize undefined node kind");
    }
}
