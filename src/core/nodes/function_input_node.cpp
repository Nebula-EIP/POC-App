#include "function_input_node.hpp"

core::FunctionInputNode::FunctionInputNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void core::FunctionInputNode::set_name(const std::string &name) {
    name_ = name;
}

const std::string &core::FunctionInputNode::name() const { return name_; }

void core::FunctionInputNode::set_type(PinDataType type) { type_ = type; }

core::NodeBase::PinDataType core::FunctionInputNode::type() const {
    return type_;
}

uint8_t core::FunctionInputNode::GetInputPinCount() const { return 0; }

uint8_t core::FunctionInputNode::GetOutputPinCount() const { return 1; }

core::NodeBase::PinDataType core::FunctionInputNode::GetInputPinType(
    uint8_t /*pin*/) const {
    return PinDataType::kUndefined;
}

core::NodeBase::PinDataType core::FunctionInputNode::GetOutputPinType(
    uint8_t pin) const {
    if (pin == 0) {
        return type_;
    }
    return PinDataType::kUndefined;
}

std::expected<void, std::string> core::FunctionInputNode::CanConnectTo(
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

std::string core::FunctionInputNode::GetInputPinName(
    uint8_t /*pin*/) const {
    return "";
}

std::string core::FunctionInputNode::GetOutputPinName(uint8_t pin) const {
    if (pin == 0) {
        return name_;
    }
    return "";
}

std::string core::FunctionInputNode::GetDisplayName() const { return name_; }

std::string core::FunctionInputNode::GetCategory() const {
    return "Functions";
}

nlohmann::json core::FunctionInputNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    json["name"] = name_;
    return json;
}

std::expected<void, std::string> core::FunctionInputNode::Deserialize(
    const nlohmann::json &json) {
    if (!json.contains("type") || !json.contains("name")) {
        return std::unexpected(
            "Missing required fields for FunctionInputNode: type or name");
    }

    try {
        type_ = StringToPinDataType(json["type"].get<std::string>());
        name_ = json["name"].get<std::string>();
        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize FunctionInputNode: ") +
            e.what());
    }
}
