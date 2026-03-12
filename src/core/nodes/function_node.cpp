#include "function_node.hpp"

#include "../graph.hpp"
#include "function_input_node.hpp"
#include "function_node_errors.hpp"

// ── Construction ────────────────────────────────────────────────────────────

core::FunctionNode::~FunctionNode() = default;

core::FunctionNode::FunctionNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind), body_(std::make_unique<Graph>()) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

// ── Name ────────────────────────────────────────────────────────────────────

void core::FunctionNode::set_name(const std::string &name) { name_ = name; }

const std::string &core::FunctionNode::name() const { return name_; }

// ── Return type ─────────────────────────────────────────────────────────────

void core::FunctionNode::set_return_type(PinDataType type) {
    return_type_ = type;
}

core::NodeBase::PinDataType core::FunctionNode::return_type() const {
    return return_type_;
}

// ── Parameters ──────────────────────────────────────────────────────────────

void core::FunctionNode::AddParameter(const std::string &name,
                                      PinDataType type) {
    auto *input_node =
        body_->AddNode<FunctionInputNode>(NodeKind::kFunctionInput);
    if (!input_node) {
        throw function_node_errors::ParameterNodeCreationException(
            "Failed to create FunctionInputNode for parameter");
    }
    input_node->set_name(name);
    input_node->set_type(type);
    parameters_.push_back({name, type, input_node->id()});
    // Resize connection vectors to match new pin count
    parents_.resize(GetInputPinCount());
}

void core::FunctionNode::RemoveParameter(uint8_t index) {
    if (index >= parameters_.size()) return;
    if (auto *node = body_->GetNode(parameters_[index].node_id)) {
        body_->RemoveNode(node);
    }
    parameters_.erase(parameters_.begin() + index);
    // Rebuild connections – the caller should have unlinked beforehand
    parents_.resize(GetInputPinCount());
}

void core::FunctionNode::RemoveParameter(const std::string &name) {
    auto it = std::find_if(
        parameters_.begin(), parameters_.end(),
        [&name](const FunctionParameter &param) { return param.name == name; });
    if (it != parameters_.end()) {
        if (auto *node = body_->GetNode(it->node_id)) {
            body_->RemoveNode(node);
        }
        parameters_.erase(it);
        // Rebuild connections – the caller should have unlinked beforehand
        parents_.resize(GetInputPinCount());
    }
}

const std::vector<core::FunctionParameter> &core::FunctionNode::parameters()
    const {
    return parameters_;
}

// ── Inner graph ─────────────────────────────────────────────────────────────

core::Graph &core::FunctionNode::body() { return *body_; }

const core::Graph &core::FunctionNode::body() const { return *body_; }

// ── Pin counts ──────────────────────────────────────────────────────────────

uint8_t core::FunctionNode::GetInputPinCount() const {
    return static_cast<uint8_t>(parameters_.size());
}

uint8_t core::FunctionNode::GetOutputPinCount() const { return 1; }

// ── Pin types ───────────────────────────────────────────────────────────────

core::NodeBase::PinDataType core::FunctionNode::GetInputPinType(
    uint8_t pin) const {
    if (pin < parameters_.size()) {
        return parameters_[pin].type;
    }
    return PinDataType::kUndefined;
}

core::NodeBase::PinDataType core::FunctionNode::GetOutputPinType(
    uint8_t pin) const {
    if (pin == 0) {
        return return_type_;
    }
    return PinDataType::kUndefined;
}

// ── Connection validation ───────────────────────────────────────────────────

std::expected<void, std::string> core::FunctionNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const {
    if (out_pin != 0) {
        return std::unexpected("Pin does not exist");
    }

    if (in_pin >= target->GetInputPinCount()) {
        return std::unexpected("Target pin does not exist");
    }

    if (GetOutputPinType(out_pin) != target->GetInputPinType(in_pin)) {
        return std::unexpected("Types don't match");
    }

    return {};
}

// ── Pin names ───────────────────────────────────────────────────────────────

std::string core::FunctionNode::GetInputPinName(uint8_t pin) const {
    if (pin < parameters_.size()) {
        return parameters_[pin].name;
    }
    return "";
}

std::string core::FunctionNode::GetOutputPinName(uint8_t pin) const {
    if (pin == 0) {
        return "Result";
    }
    return "";
}

// ── Display ─────────────────────────────────────────────────────────────────

std::string core::FunctionNode::GetDisplayName() const { return name_; }

std::string core::FunctionNode::GetCategory() const { return "Functions"; }

// ── Serialization ───────────────────────────────────────────────────────────

nlohmann::json core::FunctionNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["name"] = name_;
    json["return_type"] = core::PinDataTypeToString(return_type_);

    // Serialize parameters
    nlohmann::json params = nlohmann::json::array();
    for (const auto &param : parameters_) {
        nlohmann::json p;
        p["name"] = param.name;
        p["type"] = core::PinDataTypeToString(param.type);
        p["node_id"] = param.node_id;
        params.push_back(p);
    }
    json["parameters"] = params;

    // Serialize the inner graph (function body)
    json["body"] = body_->Serialize();

    return json;
}

std::expected<void, std::string> core::FunctionNode::Deserialize(
    const nlohmann::json &json) {
    // Validate required fields
    if (!json.contains("name") || !json.contains("return_type") ||
        !json.contains("parameters")) {
        return std::unexpected(
            "Missing required fields for FunctionNode: "
            "name, return_type, or parameters");
    }

    try {
        name_ = json["name"].get<std::string>();
        return_type_ =
            StringToPinDataType(json["return_type"].get<std::string>());

        // Deserialize parameters
        parameters_.clear();
        const auto &params = json["parameters"];
        if (!params.is_array()) {
            return std::unexpected("FunctionNode 'parameters' is not an array");
        }
        for (const auto &p : params) {
            if (!p.contains("name") || !p.contains("type")) {
                return std::unexpected(
                    "Parameter missing required fields: name or type");
            }
            FunctionParameter param;
            param.name = p["name"].get<std::string>();
            param.type = StringToPinDataType(p["type"].get<std::string>());
            if (p.contains("node_id")) {
                param.node_id = p["node_id"].get<uint32_t>();
            }
            parameters_.push_back(param);
        }

        // Resize connection vectors to match parameter count
        parents_.resize(GetInputPinCount());
        childrens_.resize(GetOutputPinCount());

        // Deserialize inner graph if present
        if (json.contains("body")) {
            auto graph_result = Graph::Deserialize(json["body"]);
            if (!graph_result) {
                return std::unexpected(
                    "Failed to deserialize FunctionNode body: " +
                    graph_result.error());
            }
            body_ = std::make_unique<Graph>(std::move(graph_result.value()));
        }

        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize FunctionNode: ") + e.what());
    }
}
