#include "literal_node.hpp"

core::LiteralNode::LiteralNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {}

uint8_t core::LiteralNode::GetInputPinCount() const { return 0; }

uint8_t core::LiteralNode::GetOutputPinCount() const { return 1; }

core::NodeBase::PinDataType core::LiteralNode::GetInputPinType(
    uint8_t /*pin*/) const {
    return PinDataType::kUndefined;
}

core::NodeBase::PinDataType core::LiteralNode::GetOutputPinType(
    uint8_t pin) const {
    if (pin == 0) {
        return type_;
    } else {
        return PinDataType::kUndefined;
    }
}

std::expected<void, std::string> core::LiteralNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const {
    if (GetOutputPinType(out_pin) != target->GetInputPinType(in_pin)) {
        return std::unexpected("Types don't match");
    }

    return {};
}

std::string core::LiteralNode::GetInputPinName(uint8_t /*pin*/) const {
    return "";
}

std::string core::LiteralNode::GetOutputPinName(uint8_t pin) const {
    if (pin == 0) {
        return "Kakou";
    } else {
        return "";
    }
}

std::string core::LiteralNode::GetDisplayName() const { return name_; }

std::string core::LiteralNode::GetCategory() const { return "Cool stuff~"; }
