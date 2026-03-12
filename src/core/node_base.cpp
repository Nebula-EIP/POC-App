#include "node_base.hpp"

#include <algorithm>

#include "nodes/function_input_node.hpp"
#include "nodes/function_node.hpp"
#include "nodes/function_output_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/printf_node.hpp"
#include "nodes/variable_node.hpp"

core::NodeBase::Connection::Connection(NodeBase *n, uint8_t p, PinDataType t)
    : node(n), pin(p), type(t) {}

bool core::NodeBase::Connection::IsConnected() const { return node != nullptr; }

core::NodeBase::NodeBase(uint32_t id, NodeKind kind) : id_(id), kind_(kind) {}

core::NodeBase::~NodeBase() = default;

uint32_t core::NodeBase::id() const { return id_; }

core::NodeBase::NodeKind core::NodeBase::kind() const { return kind_; }

// parents_ vector already filled by the Graph class
const core::NodeBase::Connection *core::NodeBase::parent(uint8_t in_pin) const {
    if (GetInputPinCount() <= in_pin || parents_.size() <= in_pin) {
        return nullptr;
    }

    const Connection &conn = parents_[in_pin];
    return conn.IsConnected() ? &conn : nullptr;
}

const std::vector<core::NodeBase::Connection> core::NodeBase::GetParents() const {
    std::vector<Connection> connected_parents;
    for (const auto& conn : parents_) {
        if (conn.IsConnected())
            connected_parents.push_back(conn);
    }
    return connected_parents;
}

// childrens_ vector already filled by the Graph class
const std::vector<core::NodeBase::Connection> &core::NodeBase::childrens(
    uint8_t out_pin) const {
    static const std::vector<NodeBase::Connection> kEmptyVector;

    if (GetOutputPinCount() <= out_pin || childrens_.size() <= out_pin) {
        return kEmptyVector;
    }

    return childrens_[out_pin];
}

void core::NodeBase::SetParent(uint8_t in_pin, NodeBase *parent,
                               uint8_t parent_pin) {
    if (parent == nullptr || GetInputPinCount() <= in_pin ||
        parent->GetOutputPinCount() <= parent_pin) {
        return;
    }

    if (parents_.size() <= in_pin) {
        parents_.resize(in_pin + 1);
    }

    parents_[in_pin].node = parent;
    parents_[in_pin].pin = parent_pin;
    parents_[in_pin].type = parent->GetOutputPinType(parent_pin);
}

void core::NodeBase::AddChild(uint8_t out_pin, NodeBase *child,
                              uint8_t child_pin) {
    if (child == nullptr || GetOutputPinCount() <= out_pin ||
        child->GetInputPinCount() <= child_pin) {
        return;
    }

    if (childrens_.size() <= out_pin) {
        childrens_.resize(out_pin + 1);
    }

    auto &children = childrens_[out_pin];
    auto reusable = std::find_if(
        children.begin(), children.end(),
        [](const Connection &connection) { return !connection.IsConnected(); });

    if (reusable != children.end()) {
        reusable->node = child;
        reusable->pin = child_pin;
        reusable->type = GetOutputPinType(out_pin);
        return;
    }

    children.emplace_back(child, child_pin, GetOutputPinType(out_pin));
}

void core::NodeBase::ClearParent(uint8_t pin) {
    if (GetInputPinCount() <= pin || parents_.size() <= pin) {
        return;
    }

    parents_[pin].node = nullptr;
    parents_[pin].pin = 0;
    parents_[pin].type = PinDataType::kUndefined;
}

void core::NodeBase::RemoveChild(uint8_t out_pin, const NodeBase *node,
                                 uint8_t in_pin) {
    if (node == nullptr || GetOutputPinCount() <= out_pin ||
        childrens_.size() <= out_pin) {
        return;
    }

    auto &children = childrens_[out_pin];
    for (auto &connection : children) {
        if (connection.node == node && connection.pin == in_pin) {
            connection.node = nullptr;
            connection.pin = 0;
            connection.type = PinDataType::kUndefined;
            break;
        }
    }
}

void core::NodeBase::InitializeConnections() {
    parents_.clear();
    parents_.resize(GetInputPinCount());

    childrens_.clear();
    childrens_.resize(GetOutputPinCount());
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
        case core::NodeBase::NodeKind::kPrintf:
            return "Printf";
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

core::NodeBase::NodeKind core::StringToNodeKind(const std::string &str) {
    if (str == "Literal") {
        return core::NodeBase::NodeKind::kLiteral;
    }
    if (str == "Variable") {
        return core::NodeBase::NodeKind::kVariable;
    }
    if (str == "Function") {
        return core::NodeBase::NodeKind::kFunction;
    }
    if (str == "FunctionInput") {
        return core::NodeBase::NodeKind::kFunctionInput;
    }
    if (str == "FunctionOutput") {
        return core::NodeBase::NodeKind::kFunctionOutput;
    }
    if (str == "Printf") {
        return core::NodeBase::NodeKind::kPrintf;
    }
    if (str == "Operator") {
        return core::NodeBase::NodeKind::kOperator;
    }
    if (str == "Condition") {
        return core::NodeBase::NodeKind::kCondition;
    }
    if (str == "Loop") {
        return core::NodeBase::NodeKind::kLoop;
    }
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

core::NodeBase::PinDataType core::StringToPinDataType(const std::string &str) {
    if (str == "Int") return core::NodeBase::PinDataType::kInt;
    if (str == "Float") return core::NodeBase::PinDataType::kFloat;
    if (str == "Bool") return core::NodeBase::PinDataType::kBool;
    if (str == "String") return core::NodeBase::PinDataType::kString;
    if (str == "Void") return core::NodeBase::PinDataType::kVoid;
    return core::NodeBase::PinDataType::kUndefined;
}

std::expected<std::unique_ptr<core::NodeBase>, std::string>
core::NodeBase::DeserializeFactory(const nlohmann::json &json,
                                   Graph * /*graph*/) {
    // Validate required fields
    if (!json.contains("id") || !json.contains("kind")) {
        return std::unexpected("Missing required fields: id or kind");
    }

    uint32_t id;
    std::string kind_str;

    try {
        id = json["id"].get<uint32_t>();
        kind_str = json["kind"].get<std::string>();
    } catch (const std::exception &e) {
        return std::unexpected(std::string("Failed to parse node fields: ") +
                               e.what());
    }

    NodeKind kind = StringToNodeKind(kind_str);
    if (kind == NodeKind::kUndefined) {
        return std::unexpected(std::string("Unknown node kind: ") + kind_str);
    }

    // Create the appropriate node type
    std::unique_ptr<NodeBase> node;
    switch (kind) {
        case NodeKind::kLiteral: {
            auto literal_node =
                std::unique_ptr<LiteralNode>(new LiteralNode(id, kind));
            // Deserialize the node's data
            auto result = literal_node->Deserialize(json);
            if (!result) {
                return std::unexpected(result.error());
            }
            // Initialize connections after deserialization
            literal_node->InitializeConnections();
            node = std::move(literal_node);
            break;
        }

        case NodeKind::kVariable: {
            auto variable_node =
                std::unique_ptr<VariableNode>(new VariableNode(id, kind));
            // Deserialize the node's data
            auto result = variable_node->Deserialize(json);
            if (!result) {
                return std::unexpected(result.error());
            }
            // Initialize connections after deserialization
            variable_node->InitializeConnections();
            node = std::move(variable_node);
            break;
        }

        case NodeKind::kFunction: {
            auto function_node =
                std::unique_ptr<FunctionNode>(new FunctionNode(id, kind));
            auto result = function_node->Deserialize(json);
            if (!result) {
                return std::unexpected(result.error());
            }
            function_node->InitializeConnections();
            node = std::move(function_node);
            break;
        }

        case NodeKind::kFunctionInput: {
            auto input_node = std::unique_ptr<FunctionInputNode>(
                new FunctionInputNode(id, kind));
            auto result = input_node->Deserialize(json);
            if (!result) {
                return std::unexpected(result.error());
            }
            input_node->InitializeConnections();
            node = std::move(input_node);
            break;
        }

        case NodeKind::kFunctionOutput: {
            auto output_node = std::unique_ptr<FunctionOutputNode>(
                new FunctionOutputNode(id, kind));
            auto result = output_node->Deserialize(json);
            if (!result) {
                return std::unexpected(result.error());
            }
            output_node->InitializeConnections();
            node = std::move(output_node);
            break;
        }

        case NodeKind::kPrintf: {
            auto printf_node =
                std::unique_ptr<PrintfNode>(new PrintfNode(id, kind));
            auto result = printf_node->Deserialize(json);
            if (!result) {
                return std::unexpected(result.error());
            }
            printf_node->InitializeConnections();
            node = std::move(printf_node);
            break;
        }

        case NodeKind::kOperator:
        case NodeKind::kCondition:
        case NodeKind::kLoop:
            return std::unexpected(
                std::string("Node kind not yet implemented: ") + kind_str);

        case NodeKind::kUndefined:
        default:
            return std::unexpected("Cannot deserialize undefined node kind");
    }

    return node;
}
