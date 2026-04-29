#include "function_node.hpp"

#include "../connection_exceptions.hpp"
#include "../graph.hpp"
#include "function_input_node.hpp"
#include "function_node_exceptions.hpp"

// ── Construction ────────────────────────────────────────────────────────────

core::FunctionNode::~FunctionNode() = default;

core::FunctionNode::FunctionNode(uint32_t id, NodeKind kind,
                                 std::pair<float, float> position) noexcept
    : NodeBase(id, kind, position), body_(std::make_unique<Graph>()) {
    InitializeConnections();
}

void core::FunctionNode::InitializeConnections() {
    parents_.clear();
    childrens_.clear();
    in_pin_id_manager_ = utils::IdManager<uint8_t>();
    out_pin_id_manager_ = utils::IdManager<uint8_t>();

    for (auto &param : parameters_) {
        AddInputPin(param.name, param.type);
        param.pin_id = parents_.back().in_pin;
    }

    AddOutputPin("Result", return_type_);
}

// ── Name ────────────────────────────────────────────────────────────────────

void core::FunctionNode::SetName(const std::string &name) { name_ = name; }

const std::string &core::FunctionNode::Name() const noexcept { return name_; }

// ── Return type ─────────────────────────────────────────────────────────────

void core::FunctionNode::set_return_type(PinDataType type) {
    return_type_ = type;

    if (GetOutputPinCount() == 0) {
        AddOutputPin("Result", return_type_);
        return;
    }

    auto out_pin_id = childrens_.front().id;
    if (IsOutputPinConnected(out_pin_id)) {
        THROW_EXCEPTION(PinStillConnectedException,
                        "Output pin n°{} is still connected", out_pin_id);
    }

    childrens_.front().type = return_type_;
}

core::NodeBase::PinDataType core::FunctionNode::return_type() const noexcept {
    return return_type_;
}

// ── Parameters ──────────────────────────────────────────────────────────────

void core::FunctionNode::AddParameter(const std::string &name,
                                      PinDataType type) {
    auto *input_node =
        body_->AddNode<FunctionInputNode>(NodeKind::kFunctionInput, {0, 0});

    if (!input_node) {
        THROW_EXCEPTION(FunctionNodeException,
                        "Failed to create FunctionInputNode for parameter");
    }
    input_node->SetName(name);
    input_node->SetType(type);
    parameters_.push_back({name, type, 0, input_node->id()});
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

const std::vector<core::FunctionParameter> &core::FunctionNode::Parameters()
    const noexcept {
    return parameters_;
}

// ── Inner graph ─────────────────────────────────────────────────────────────

core::Graph &core::FunctionNode::body() { return *body_; }

const core::Graph &core::FunctionNode::body() const { return *body_; }

// ── Connection validation ───────────────────────────────────────────────────

std::expected<void, std::string> core::FunctionNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const noexcept {
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

// ── Display ─────────────────────────────────────────────────────────────────

std::string core::FunctionNode::GetDisplayName() const noexcept {
    return name_;
}

std::string core::FunctionNode::GetCategory() const noexcept {
    return "Functions";
}

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
        p["pin_id"] = param.pin_id;
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
            if (p.contains("pin_id")) {
                param.pin_id = p["pin_id"].get<uint8_t>();
            }
            if (p.contains("node_id")) {
                param.node_id = p["node_id"].get<uint32_t>();
            }
            parameters_.push_back(param);
        }

        InitializeConnections();

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
