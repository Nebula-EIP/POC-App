#include "for_node.hpp"

#include "../graph.hpp"

namespace core {

ForNode::ForNode(uint32_t id, NodeKind kind,
                 utils::WrappedVector2 position) noexcept
    : NodeBase(id, kind, position) {
    InitializeConnections();
}

void ForNode::InitializeConnections() {
    AddInputPin("init", PinDataType::kInt);
    AddInputPin("cond", PinDataType::kBool);
    AddInputPin("step", PinDataType::kInt);
    AddOutputPin("body", PinDataType::kVoid);
}

void ForNode::SetName(const std::string &name) { name_ = name; }
const std::string &ForNode::Name() const noexcept { return name_; }

uint8_t ForNode::GetInputPinCount() const noexcept { return 3; }
uint8_t ForNode::GetOutputPinCount() const noexcept { return 1; }

NodeBase::PinDataType ForNode::GetInputPinType(uint8_t pin) const {
    switch (pin) {
        case 0:
        case 2:
            return PinDataType::kInt;
        case 1:
            return PinDataType::kBool;
        default:
            return PinDataType::kUndefined;
    }
}

NodeBase::PinDataType ForNode::GetOutputPinType(uint8_t pin) const {
    return PinDataType::kVoid;
}

std::expected<void, std::string> ForNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const noexcept {
    if (!target) return std::unexpected(std::string("Target node is null"));
    if (GetOutputPinType(out_pin) != target->GetInputPinType(in_pin)) {
        return std::unexpected(std::string("Incompatible pin types for ForNode output"));
    }
    return {};
}

std::string ForNode::GetDisplayName() const noexcept { return "For"; }
std::string ForNode::GetCategory() const noexcept { return "Control"; }

nlohmann::json ForNode::Serialize() const {
    nlohmann::json j;
    j["id"] = id();
    j["kind"] = NodeKindToString(kind());
    j["name"] = name_;
    return j;
}

std::expected<void, std::string> ForNode::Deserialize(
    const nlohmann::json &json) {
    if (json.contains("name") && json["name"].is_string()) {
        name_ = json["name"].get<std::string>();
    }
    return {};
}

}  // namespace core
