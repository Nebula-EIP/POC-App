#include "condition_node.hpp"

core::ConditionNode::ConditionNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void core::ConditionNode::set_name(const std::string &name) { name_ = name; }

const std::string &core::ConditionNode::name() const { return name_; }

uint8_t core::ConditionNode::GetInputPinCount() const { return 1; }

uint8_t core::ConditionNode::GetOutputPinCount() const { return 1; }

core::NodeBase::PinDataType core::ConditionNode::GetInputPinType(
    uint8_t /*pin*/) const {
    return PinDataType::kBool;
}

core::NodeBase::PinDataType core::ConditionNode::GetOutputPinType(
    uint8_t pin) const {
    if (pin <= 2) {
        return PinDataType::kVoid;
    } else {
        return PinDataType::kUndefined;
    }
}

std::expected<void, std::string> core::ConditionNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const {
    if (out_pin > 2) {
        return std::unexpected("Pin does not exists");
    }

    if (in_pin >= target->GetInputPinCount()) {
        return std::unexpected("Target pin does not exists");
    }

    if (GetOutputPinType(out_pin) != target->GetInputPinType(in_pin)) {
        return std::unexpected("Types don't match");
    }

    return {};
}

std::string core::ConditionNode::GetInputPinName(uint8_t pin) const {
    if (pin == 0) {
        return "Condition";
    } else {
        return "";
    }
}

std::string core::ConditionNode::GetOutputPinName(uint8_t pin) const {
    switch (pin) {
        case 0: return "If";
        case 1: return "Else";
        case 2: return "Then";
        default: return "";
    }
}

std::string core::ConditionNode::GetDisplayName() const { return name_; }

std::string core::ConditionNode::GetCategory() const { return "Branch"; }

nlohmann::json core::ConditionNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["name"] = name_;
    return json;
}

std::expected<void, std::string> core::ConditionNode::Deserialize(
    const nlohmann::json &json) {
    // Validate required fields for ConditionNode
    if (!json.contains("name")) {
        return std::unexpected(
            "Missing required fields for ConditionNode: name");
    }

    try {
        std::string name = json["name"].get<std::string>();

        name_ = name;

        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize ConditionNode: ") + e.what());
    }
}
