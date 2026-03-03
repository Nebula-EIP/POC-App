#include "literal_node.hpp"

using namespace core;

LiteralNode::LiteralNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {}

uint8_t LiteralNode::GetInputPinCount() const {
    return 0;
}

uint8_t LiteralNode::GetOutputPinCount() const {
    return 1;
}

NodeBase::PinDataType LiteralNode::GetInputPinType(uint8_t /*pin*/) const {
    return PinDataType::kUndefined;
}

NodeBase::PinDataType LiteralNode::GetOutputPinType(uint8_t pin) const {
    if (pin == 0)
        return type_;
    else
        return PinDataType::kUndefined;
}

std::expected<void, std::string> LiteralNode::CanConnectTo(uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const {
    if (GetOutputPinType(out_pin) != target->GetInputPinType(in_pin))
        return std::unexpected("Types don't match");

    return {};
}

std::string LiteralNode::GetInputPinName(uint8_t /*pin*/) const {
    return "";
}

std::string LiteralNode::GetOutputPinName(uint8_t pin) const {
    if (pin == 0)
        return "Kakou";
    else
        return "";
}

std::string LiteralNode::GetDisplayName() const {
    return name_;
}

std::string LiteralNode::GetCategory() const {
    return "Cool stuff~";
}

