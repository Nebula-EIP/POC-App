#include "literal_node.hpp"

core::LiteralNode::LiteralNode(uint32_t id, NodeKind kind)
    : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

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

nlohmann::json core::LiteralNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    json["name"] = name_;
    return json;
}

std::unique_ptr<core::LiteralNode> core::LiteralNode::DeserializeHelper(
    const nlohmann::json &json, uint32_t id) {
    // Validate required fields for LiteralNode
    if (!json.contains("type") || !json.contains("name")) {
        return nullptr;
    }

    try {
        std::string type_str = json["type"].get<std::string>();
        std::string name = json["name"].get<std::string>();

        auto node = std::unique_ptr<LiteralNode>(
            new LiteralNode(id, NodeKind::kLiteral));
        node->type_ = StringToPinDataType(type_str);
        node->name_ = name;

        return node;
    } catch (const std::exception &) {
        return nullptr;
    }
}
