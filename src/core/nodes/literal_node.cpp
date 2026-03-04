#include "literal_node.hpp"

core::LiteralNode::LiteralNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void core::LiteralNode::set_name(const std::string &name) { name_ = name; }

const std::string &core::LiteralNode::name() const { return name_; }

void core::LiteralNode::set_type(PinDataType type) { type_ = type; }

core::NodeBase::PinDataType core::LiteralNode::type() const { return type_; }

void core::LiteralNode::set_data(std::any data) { data_ = data; }

std::any core::LiteralNode::data() const { return data_; }

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
    if (out_pin != 0) {
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
