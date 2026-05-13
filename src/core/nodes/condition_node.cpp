#include "condition_node.hpp"

#include "../graph.hpp"

namespace core {

ConditionNode::ConditionNode(uint32_t id, NodeKind kind,
                             utils::WrappedVector2 position) noexcept
    : NodeBase(id, kind, position) {
    InitializeConnections();
}

void ConditionNode::InitializeConnections() {
    // Input: condition (bool)
    AddInputPin("cond", PinDataType::kBool);
    // Output 0: true branch (execution)
    AddOutputPin("true", PinDataType::kVoid);
    // Output 1: false branch (execution)
    AddOutputPin("false", PinDataType::kVoid);
}

void ConditionNode::SetName(const std::string &name) { name_ = name; }
const std::string &ConditionNode::Name() const noexcept { return name_; }

uint8_t ConditionNode::GetInputPinCount() const noexcept { return 1; }
uint8_t ConditionNode::GetOutputPinCount() const noexcept { return 2; }

NodeBase::PinDataType ConditionNode::GetInputPinType(uint8_t pin) const {
    return PinDataType::kBool;
}
NodeBase::PinDataType ConditionNode::GetOutputPinType(uint8_t pin) const {
    return PinDataType::kVoid;
}

std::expected<void, std::string> ConditionNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const noexcept {
    if (!target) return std::unexpected(std::string("Target node is null"));
    // allow connections where types match or target expects anything
    if (target->GetInputPinType(in_pin) != GetOutputPinType(out_pin)) {
        return std::unexpected(
            std::string("Incompatible pin types for ConditionNode output"));
    }
    return {};
}

std::string ConditionNode::GetDisplayName() const noexcept { return name_; }
std::string ConditionNode::GetCategory() const noexcept { return "Control"; }

nlohmann::json ConditionNode::Serialize() const {
    nlohmann::json j;
    j["id"] = id();
    j["kind"] = NodeKindToString(kind());
    j["name"] = name_;
    return j;
}

std::expected<void, std::string> ConditionNode::Deserialize(
    const nlohmann::json &json) {
    if (json.contains("name") && json["name"].is_string()) {
        name_ = json["name"].get<std::string>();
    }
    return {};
}

}  // namespace core
