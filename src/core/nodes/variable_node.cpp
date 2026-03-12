#include "variable_node.hpp"
#include "../connection_exceptions.hpp"

core::VariableNode::VariableNode(uint32_t id, NodeKind kind) noexcept
    : NodeBase(id, kind) {
    InitializeConnections();
}

void core::VariableNode::InitializeConnections() {
    // VariableNode has 1 input pin and 1 output pin
    parents_.resize(1);
    parents_[0] = Connection(nullptr, 0, in_pin_id_manager_.NewId(), PinDataType::kInt);
    childrens_.resize(1);
    uint32_t id = out_pin_id_manager_.NewId();
    std::get<0>(childrens_[0]) = id;
}

void core::VariableNode::set_name(const std::string &name) { name_ = name; }

const std::string &core::VariableNode::name() const noexcept { return name_; }

void core::VariableNode::set_type(PinDataType type) {
    // Check for still connected pins
    for (auto child : GetAllChildrens()) {
        if (child.IsConnected()) {
            THROW_EXCEPTION(PinStillConnectedException, "Output pin n°{} is still connected",
                child.out_pin);
        }
    }

    for (auto parent : GetAllParents()) {
        if (parent.IsConnected()) {
            THROW_EXCEPTION(PinStillConnectedException, "Input pin n°{} is still connected",
                parent.in_pin);
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

core::NodeBase::PinDataType core::VariableNode::type() const noexcept { return type_; }

void core::VariableNode::set_data(std::any data) { data_ = data; }

std::any core::VariableNode::data() const noexcept { return data_; }

uint8_t core::VariableNode::GetInputPinCount() const noexcept { return 1; }

uint8_t core::VariableNode::GetOutputPinCount() const noexcept { return 1; }

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

std::string core::VariableNode::GetDisplayName() const noexcept { return name_; }

std::string core::VariableNode::GetCategory() const noexcept { return "Cool stuff~"; }

nlohmann::json core::VariableNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    json["name"] = name_;
    return json;
}

std::expected<void, std::string> core::VariableNode::Deserialize(
    const nlohmann::json &json) {
    // Validate required fields for VariableNode
    if (!json.contains("type") || !json.contains("name")) {
        return std::unexpected(
            "Missing required fields for VariableNode: type or name");
    }

    try {
        std::string type_str = json["type"].get<std::string>();
        std::string name = json["name"].get<std::string>();

        type_ = StringToPinDataType(type_str);
        name_ = name;

        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize VariableNode: ") + e.what());
    }
}
