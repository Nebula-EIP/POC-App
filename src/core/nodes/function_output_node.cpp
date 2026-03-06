#include "function_output_node.hpp"

core::FunctionOutputNode::FunctionOutputNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void core::FunctionOutputNode::set_name(const std::string &name) {
    name_ = name;
}

const std::string &core::FunctionOutputNode::name() const { return name_; }

void core::FunctionOutputNode::set_type(PinDataType type) { type_ = type; }

core::NodeBase::PinDataType core::FunctionOutputNode::type() const {
    return type_;
}

uint8_t core::FunctionOutputNode::GetInputPinCount() const { return 1; }

uint8_t core::FunctionOutputNode::GetOutputPinCount() const { return 0; }

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
    uint8_t /*in_pin*/) const {
    return std::unexpected("FunctionOutputNode has no output pins");
}

std::string core::FunctionOutputNode::GetInputPinName(uint8_t pin) const {
    if (pin == 0) {
        return name_;
    }
    return "";
}

std::string core::FunctionOutputNode::GetOutputPinName(
    uint8_t /*pin*/) const {
    return "";
}

std::string core::FunctionOutputNode::GetDisplayName() const { return name_; }

std::string core::FunctionOutputNode::GetCategory() const {
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
