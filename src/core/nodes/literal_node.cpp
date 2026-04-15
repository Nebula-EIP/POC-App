#include "literal_node.hpp"

#include "../connection_exceptions.hpp"

core::LiteralNode::LiteralNode(uint32_t id, NodeKind kind, std::pair<float, float> position) noexcept
    : NodeBase(id, kind, position) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void core::LiteralNode::InitializeConnections() {
    // LiteralNode has 0 input pins and 1 output pin
    childrens_.resize(1);
    uint32_t id = out_pin_id_manager_.NewId();
    std::get<0>(childrens_[0]) = id;
}

void core::LiteralNode::set_name(const std::string &name) { name_ = name; }

const std::string &core::LiteralNode::name() const noexcept { return name_; }

void core::LiteralNode::set_type(PinDataType type) {
    // Check for still connected pins
    for (auto child : GetAllChildrens()) {
        if (child.IsConnected()) {
            THROW_EXCEPTION(PinStillConnectedException,
                            "Output pin n°{} is still connected",
                            child.out_pin);
        }
    }

    for (auto parent : GetAllParents()) {
        if (parent.IsConnected()) {
            THROW_EXCEPTION(PinStillConnectedException,
                            "Input pin n°{} is still connected", parent.in_pin);
        }
    }

    // Update pins types
    for (auto &pin : childrens_) {
        for (auto &child : std::get<1>(pin)) {
            child.type = type;
        }
    }

    for (auto &parent : parents_) {
        parent.type = type;
    }

    type_ = type;
}

core::NodeBase::PinDataType core::LiteralNode::type() const noexcept {
    return type_;
}

void core::LiteralNode::set_data(std::any data) { data_ = data; }

std::any core::LiteralNode::data() const noexcept { return data_; }

uint8_t core::LiteralNode::GetInputPinCount() const noexcept { return 0; }

uint8_t core::LiteralNode::GetOutputPinCount() const noexcept { return 1; }

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
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const noexcept {
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

std::string core::LiteralNode::GetDisplayName() const noexcept { return name_; }

std::string core::LiteralNode::GetCategory() const noexcept {
    return "Cool stuff~";
}

nlohmann::json core::LiteralNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    json["name"] = name_;
    return json;
}

std::expected<void, std::string> core::LiteralNode::Deserialize(
    const nlohmann::json &json) {
    // Validate required fields for LiteralNode
    if (!json.contains("type") || !json.contains("name")) {
        return std::unexpected(
            "Missing required fields for LiteralNode: type or name");
    }

    try {
        std::string type_str = json["type"].get<std::string>();
        std::string name = json["name"].get<std::string>();

        type_ = StringToPinDataType(type_str);
        name_ = name;

        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize LiteralNode: ") + e.what());
    }
}
