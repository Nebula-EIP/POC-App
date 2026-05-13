#include "loop_node.hpp"

#include "../graph.hpp"

namespace core {

LoopNode::LoopNode(uint32_t id, NodeKind kind, utils::WrappedVector2 position) noexcept
    : NodeBase(id, kind, position) {
    InitializeConnections();
}

void LoopNode::InitializeConnections() {
    // Input: condition (bool)
    AddInputPin("cond", PinDataType::kBool);
    // Output 0: body (execution)
    AddOutputPin("body", PinDataType::kVoid);
}

void LoopNode::SetName(const std::string &name) { name_ = name; }
const std::string &LoopNode::Name() const noexcept { return name_; }

uint8_t LoopNode::GetInputPinCount() const noexcept { return 1; }
uint8_t LoopNode::GetOutputPinCount() const noexcept { return 1; }

NodeBase::PinDataType LoopNode::GetInputPinType(uint8_t pin) const { return PinDataType::kBool; }
NodeBase::PinDataType LoopNode::GetOutputPinType(uint8_t pin) const { return PinDataType::kVoid; }

std::expected<void, std::string> LoopNode::CanConnectTo(uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const noexcept {
    if (!target) return std::unexpected(std::string("Target node is null"));
    if (target->GetInputPinType(in_pin) != GetOutputPinType(out_pin)) {
        return std::unexpected(std::string("Incompatible pin types for LoopNode output"));
    }
    return {};
}

std::string LoopNode::GetDisplayName() const noexcept { return name_; }
std::string LoopNode::GetCategory() const noexcept { return "Control"; }

nlohmann::json LoopNode::Serialize() const {
    nlohmann::json j;
    j["id"] = id();
    j["kind"] = NodeKindToString(kind());
    j["name"] = name_;
    return j;
}

std::expected<void, std::string> LoopNode::Deserialize(const nlohmann::json &json) {
    if (json.contains("name") && json["name"].is_string()) {
        name_ = json["name"].get<std::string>();
    }
    return {};
}

} // namespace core
