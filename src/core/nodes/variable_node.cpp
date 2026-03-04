#include "variable_node.hpp"

core::VariableNode::VariableNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void core::VariableNode::set_name(const std::string &name) { name_ = name; }

const std::string &core::VariableNode::name() const { return name_; }

void core::VariableNode::set_type(PinDataType type) { type_ = type; }

core::NodeBase::PinDataType core::VariableNode::type() const { return type_; }

void core::VariableNode::set_data(std::any data) { data_ = data; }

std::any core::VariableNode::data() const { return data_; }

uint8_t core::VariableNode::GetInputPinCount() const { return 1; }

uint8_t core::VariableNode::GetOutputPinCount() const { return 1; }

core::NodeBase::PinDataType core::VariableNode::GetInputPinType(
    uint8_t pin) const {
    if (pin == 0) {
        return type_;
    } else {
        return PinDataType::kUndefined;
    }
}

core::NodeBase::PinDataType core::VariableNode::GetOutputPinType(
    uint8_t pin) const {
    if (pin == 0) {
        return type_;
    } else {
        return PinDataType::kUndefined;
    }
}

std::expected<void, std::string> core::VariableNode::CanConnectTo(
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

std::string core::VariableNode::GetInputPinName(uint8_t pin) const {
    if (pin == 0) {
        return "Input";
    } else {
        return "";
    }
}

std::string core::VariableNode::GetOutputPinName(uint8_t pin) const {
    if (pin == 0) {
        return "Output";
    } else {
        return "";
    }
}

std::string core::VariableNode::GetDisplayName() const { return name_; }

std::string core::VariableNode::GetCategory() const { return "Cool stuff~"; }
