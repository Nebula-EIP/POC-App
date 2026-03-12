#include "function_output_node.hpp"
#include "../connection_exceptions.hpp"

core::FunctionOutputNode::FunctionOutputNode(uint32_t id, NodeKind kind)  noexcept
    : NodeBase(id, kind) {
    InitializeConnections();
}

void core::FunctionOutputNode::InitializeConnections() {
    // FunctionOutputNode has 1 input pin and 0 output pins
    parents_.resize(1);
    childrens_.resize(0);
}

void core::FunctionOutputNode::set_name(const std::string &name) {
    name_ = name;
}

const std::string &core::FunctionOutputNode::name() const  noexcept { return name_; }

void core::FunctionOutputNode::set_type(PinDataType type) {
    // Check for still connected pins
    for (auto child : GetAllChildrens()) {
        if (child.IsConnected()) {
            THROW_EXCEPTION(PinStillConnectedException, "Output pin n°{} is still connected",
                child.out_pin);
        }
    }

    for (auto parent : GetAllParents()) {
        if (parent.IsConnected()) {
            THROW_EXCEPTION(PinStillConnectedException, "Input pin n°{} is still connected",
                parent.in_pin);
        }
    }

    // Update pins types
    for (auto &pin : childrens_) {
        for (auto &child : std::get<1>(pin)) {
            child.type = type;
        }
    }

    for (auto &parent : parents_) {
        parent.type = type;
    }
    
    type_ = type;
}

core::NodeBase::PinDataType core::FunctionOutputNode::type() const  noexcept {
    return type_;
}

uint8_t core::FunctionOutputNode::GetInputPinCount() const  noexcept { return 1; }

uint8_t core::FunctionOutputNode::GetOutputPinCount() const  noexcept { return 0; }

core::NodeBase::PinDataType core::FunctionOutputNode::GetInputPinType(
    uint8_t pin) const {
    if (pin == 0) {
        return type_;
    }
    return PinDataType::kUndefined;
}

core::NodeBase::PinDataType core::FunctionOutputNode::GetOutputPinType(
    uint8_t /*pin*/) const {
    return PinDataType::kUndefined;
}

std::expected<void, std::string> core::FunctionOutputNode::CanConnectTo(
    uint8_t /*out_pin*/, const NodeBase * /*target*/,
    uint8_t /*in_pin*/) const  noexcept {
    return std::unexpected("FunctionOutputNode has no output pins");
}

std::string core::FunctionOutputNode::GetInputPinName(uint8_t pin) const {
    if (pin == 0) {
        return name_;
    }
    return "";
}

std::string core::FunctionOutputNode::GetOutputPinName(uint8_t /*pin*/) const {
    return "";
}

std::string core::FunctionOutputNode::GetDisplayName() const  noexcept { return name_; }

std::string core::FunctionOutputNode::GetCategory() const  noexcept {
    return "Functions";
}

nlohmann::json core::FunctionOutputNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    json["name"] = name_;
    return json;
}

std::expected<void, std::string> core::FunctionOutputNode::Deserialize(
    const nlohmann::json &json) {
    if (!json.contains("type") || !json.contains("name")) {
        return std::unexpected(
            "Missing required fields for FunctionOutputNode: type or name");
    }

    try {
        type_ = StringToPinDataType(json["type"].get<std::string>());
        name_ = json["name"].get<std::string>();
        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize FunctionOutputNode: ") +
            e.what());
    }
}
