#include "node_base.hpp"

#include <algorithm>
#include <exception>

#include "connection_exceptions.hpp"
#include "nodes/function_input_node.hpp"
#include "nodes/function_node.hpp"
#include "nodes/function_output_node.hpp"
#include "nodes/literal_node.hpp"
#include "nodes/operator_node.hpp"
#include "nodes/variable_node.hpp"

core::NodeBase::Connection::Connection(NodeBase *node, uint8_t out_pin,
                                       uint8_t in_pin,
                                       PinDataType type) noexcept
    : node(node), out_pin(out_pin), in_pin(in_pin), type(type) {}

bool core::NodeBase::Connection::IsConnected() const noexcept {
    return node != nullptr;
}

core::NodeBase::NodeBase(uint32_t id, NodeKind kind) noexcept
    : id_(id), kind_(kind) {}

core::NodeBase::~NodeBase() noexcept = default;

uint32_t core::NodeBase::id() const noexcept { return id_; }

core::NodeBase::NodeKind core::NodeBase::kind() const noexcept { return kind_; }

// parents_ vector already filled by the Graph class
core::NodeBase::Connection core::NodeBase::parent(uint8_t in_pin) const {
    auto it = std::find_if(
        parents_.begin(), parents_.end(),
        [in_pin](Connection conn) { return conn.in_pin == in_pin; });

    if (it == parents_.end()) {
        THROW_EXCEPTION(InvalidPinIndexException,
                        "Input pin {} does not exists", in_pin);
    }

    return (*it);
}

const std::vector<core::NodeBase::Connection> &core::NodeBase::GetAllParents()
    const noexcept {
    return parents_;
}

// childrens_ vector already filled by the Graph class
const std::vector<core::NodeBase::Connection> *core::NodeBase::childrens(
    uint8_t out_pin) const {
    auto it = std::find_if(
        childrens_.begin(), childrens_.end(),
        [out_pin](
            std::pair<uint8_t, std::vector<core::NodeBase::Connection>> conns) {
            return std::get<0>(conns) == out_pin;
        });

    if (it == childrens_.end()) {
        THROW_EXCEPTION(InvalidPinIndexException,
                        "Output pin {} does not exists", out_pin);
    }

    return &(std::get<1>(*it));
}

const std::vector<core::NodeBase::Connection> &core::NodeBase::GetAllChildrens()
    const noexcept {
    static std::vector<Connection> childs;

    childs.clear();
    for (auto conn : childrens_) {
        childs.insert(childs.end(), std::get<1>(conn).begin(),
                      std::get<1>(conn).end());
    }
    return childs;
}

bool core::NodeBase::InputPinExists(uint8_t pin) const noexcept {
    auto it = std::find_if(
        parents_.begin(), parents_.end(),
        [pin](const Connection &conn) { return conn.in_pin == pin; });
    return it != parents_.end();
}

bool core::NodeBase::OutputPinExists(uint8_t pin) const noexcept {
    auto it = std::find_if(
        childrens_.begin(), childrens_.end(),
        [pin](const std::pair<uint8_t, std::vector<Connection>> &conns) {
            return std::get<0>(conns) == pin;
        });
    return it != childrens_.end();
}

bool core::NodeBase::IsInputPinConnected(uint8_t pin) const noexcept {
    auto it = std::find_if(
        parents_.begin(), parents_.end(),
        [pin](const Connection &conn) { return conn.in_pin == pin; });

    if (it == parents_.end()) {
        return false;
    }

    return it->IsConnected();
}

bool core::NodeBase::IsOutputPinConnected(uint8_t pin) const noexcept {
    auto it = std::find_if(
        childrens_.begin(), childrens_.end(),
        [pin](const std::pair<uint8_t, std::vector<Connection>> &conns) {
            return std::get<0>(conns) == pin;
        });

    // Pin don't exists
    if (it == childrens_.end()) {
        return false;
    }

    // Check for connections
    const auto &connections = std::get<1>(*it);
    return std::any_of(
        connections.begin(), connections.end(),
        [](const Connection &conn) { return conn.IsConnected(); });
}

// Internal API, the Graph is responsible for checking input values viability
void core::NodeBase::SetParent(uint8_t in_pin, NodeBase *parent,
                               uint8_t parent_pin) noexcept {
    auto it = std::find_if(
        parents_.begin(), parents_.end(),
        [in_pin](Connection conn) { return conn.in_pin == in_pin; });

    it->in_pin = in_pin;
    it->node = parent;
    it->out_pin = parent_pin;
}

// Internal API, the Graph is responsible for checking input values viability
void core::NodeBase::AddChild(uint8_t out_pin, NodeBase *child,
                              uint8_t child_pin) noexcept {
    auto it = std::find_if(
        childrens_.begin(), childrens_.end(),
        [out_pin](
            std::pair<uint8_t, std::vector<core::NodeBase::Connection>> conns) {
            return std::get<0>(conns) == out_pin;
        });

    Connection conn(child, out_pin, child_pin, GetOutputPinType(out_pin));
    std::get<1>(*it).push_back(conn);
}

// Internal API, the Graph is responsible for checking input values viability
void core::NodeBase::ClearParent(uint8_t pin) noexcept {
    auto it =
        std::find_if(parents_.begin(), parents_.end(),
                     [pin](Connection conn) { return conn.in_pin == pin; });

    it->node = nullptr;
    it->out_pin = 0;
}

// Internal API, the Graph is responsible for checking input values viability
void core::NodeBase::RemoveChild(uint8_t out_pin, const NodeBase *node,
                                 uint8_t in_pin) noexcept {
    auto v_it = std::find_if(
        childrens_.begin(), childrens_.end(),
        [out_pin](
            std::pair<uint8_t, std::vector<core::NodeBase::Connection>> conns) {
            return std::get<0>(conns) == out_pin;
        });

    std::vector<core::NodeBase::Connection> &vec = std::get<1>(*v_it);

    auto it = std::find_if(
        vec.begin(), vec.end(), [out_pin, node, in_pin](Connection conn) {
            return ((conn.in_pin == in_pin) && (conn.node == node) &&
                    (conn.out_pin == out_pin));
        });

    vec.erase(it);
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

        case NodeKind::kOperator: {
            auto operator_node =
                std::unique_ptr<OperatorNode>(new OperatorNode(id, kind));
            auto result = operator_node->Deserialize(json);
            if (!result) {
                return std::unexpected(result.error());
            }
            operator_node->InitializeConnections();
            node = std::move(operator_node);
            break;
        }

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
