#include "printf_node.hpp"

core::PrintfNode::PrintfNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void core::PrintfNode::set_type(PinDataType type) { type_ = type; }

core::NodeBase::PinDataType core::PrintfNode::type() const { return type_; }

uint8_t core::PrintfNode::GetInputPinCount() const { return 1; }

uint8_t core::PrintfNode::GetOutputPinCount() const { return 0; }

core::NodeBase::PinDataType core::PrintfNode::GetInputPinType(
    uint8_t pin) const {
    if (pin == 0) {
        return type_;
    } else {
        return PinDataType::kUndefined;
    }
}

core::NodeBase::PinDataType core::PrintfNode::GetOutputPinType(
    uint8_t /*pin*/) const {
    return PinDataType::kUndefined;
}

std::expected<void, std::string> core::PrintfNode::CanConnectTo(
    uint8_t /*out_pin*/, const NodeBase * /*target*/,
    uint8_t /*in_pin*/) const {
    return std::unexpected("Printf node has no output pins");
}

std::string core::PrintfNode::GetInputPinName(uint8_t pin) const {
    if (pin == 0) {
        return "Value";
    } else {
        return "";
    }
}

std::string core::PrintfNode::GetOutputPinName(uint8_t /*pin*/) const {
    return "";
}

std::string core::PrintfNode::GetDisplayName() const { return "Printf"; }

std::string core::PrintfNode::GetCategory() const { return "I/O"; }

nlohmann::json core::PrintfNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    return json;
}

std::expected<void, std::string> core::PrintfNode::Deserialize(
    const nlohmann::json &json) {
    if (!json.contains("type")) {
        return std::unexpected(
            "Missing required field for PrintfNode: type");
    }

    try {
        std::string type_str = json["type"].get<std::string>();
        type_ = StringToPinDataType(type_str);
        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize PrintfNode: ") + e.what());
    }
}
