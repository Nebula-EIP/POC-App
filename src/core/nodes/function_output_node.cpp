#include "function_output_node.hpp"

#include "../connection_exceptions.hpp"

core::FunctionOutputNode::FunctionOutputNode(uint32_t id,
                                             NodeKind kind) noexcept
    : NodeBase(id, kind) {}

void core::FunctionOutputNode::InitializeConnections() {
    parents_.clear();
    childrens_.clear();
    in_pin_id_manager_ = utils::IdManager<uint8_t>();
    out_pin_id_manager_ = utils::IdManager<uint8_t>();
    AddInputPin(name_, type_);
}

void core::FunctionOutputNode::set_name(const std::string &name) {
    name_ = name;
    if (!parents_.empty()) {
        parents_.front().in_pin_name = name_;
    }
}

const std::string &core::FunctionOutputNode::name() const noexcept {
    return name_;
}

void core::FunctionOutputNode::set_type(PinDataType type) {
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
    for (auto &parent : parents_) {
        parent.type = type;
        parent.in_pin_name = name_;
    }

    type_ = type;
}

core::NodeBase::PinDataType core::FunctionOutputNode::type() const noexcept {
    return type_;
}

std::expected<void, std::string> core::FunctionOutputNode::CanConnectTo(
    uint8_t /*out_pin*/, const NodeBase * /*target*/,
    uint8_t /*in_pin*/) const noexcept {
    return std::unexpected("FunctionOutputNode has no output pins");
}

std::string core::FunctionOutputNode::GetDisplayName() const noexcept {
    return name_;
}

std::string core::FunctionOutputNode::GetCategory() const noexcept {
    return "Functions";
}

nlohmann::json core::FunctionOutputNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = core::NodeKindToString(kind());
    json["type"] = core::PinDataTypeToString(type_);
    json["name"] = name_;
    return json;
}

std::expected<void, std::string> core::FunctionOutputNode::Deserialize(
    const nlohmann::json &json) {
    if (!json.contains("type") || !json.contains("name")) {
        return std::unexpected(
            "Missing required fields for FunctionOutputNode: type or name");
    }

    try {
        type_ = StringToPinDataType(json["type"].get<std::string>());
        name_ = json["name"].get<std::string>();
        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize FunctionOutputNode: ") +
            e.what());
    }
}
