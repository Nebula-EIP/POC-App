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

nlohmann::json core::VariableNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    json["name"] = name_;
    return json;
}

std::unique_ptr<core::VariableNode> core::VariableNode::DeserializeHelper(
    const nlohmann::json &json, uint32_t id) {
    // Validate required fields for VariableNode
    if (!json.contains("type") || !json.contains("name")) {
        return nullptr;
    }

    try {
        std::string type_str = json["type"].get<std::string>();
        std::string name = json["name"].get<std::string>();

        auto node = std::unique_ptr<VariableNode>(
            new VariableNode(id, NodeKind::kVariable));
        node->type_ = StringToPinDataType(type_str);
        node->name_ = name;

        return node;
    } catch (const std::exception &) {
        return nullptr;
    }
}
