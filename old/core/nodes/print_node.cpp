#include "print_node.hpp"

#include <expected>

#include "../graph.hpp"

namespace core {

PrintNode::PrintNode(uint32_t id, NodeKind kind,
                     utils::WrappedVector2 position) noexcept
    : NodeBase(id, kind, position) {
    InitializeConnections();
}

void PrintNode::InitializeConnections() {
    AddInputPin("control", PinDataType::kVoid);
    AddInputPin("value", PinDataType::kString);
}

void PrintNode::SetName(const std::string &name) { name_ = name; }
const std::string &PrintNode::Name() const noexcept { return name_; }

uint8_t PrintNode::GetInputPinCount() const noexcept { return 2; }

NodeBase::PinDataType PrintNode::GetInputPinType(uint8_t pin) const {
    if (pin == 0) {
        return PinDataType::kVoid;
    }
    return PinDataType::kString;
}

std::expected<void, std::string> PrintNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const noexcept {
    // Allow only string connections for now
    if (!target) {
        return std::unexpected(std::string("Target node is null"));
    }
    if (in_pin == 0) {
        if (target->GetOutputPinType(out_pin) != PinDataType::kVoid) {
            return std::unexpected(
                std::string("Incompatible pin types for PrintNode control"));
        }
        return {};
    }

    if (target->GetOutputPinType(out_pin) == PinDataType::kVoid) {
        return std::unexpected(
            std::string("Incompatible pin types for PrintNode value"));
    }
    return {};
}

std::string PrintNode::GetDisplayName() const noexcept { return name_; }
std::string PrintNode::GetCategory() const noexcept { return "I/O"; }

nlohmann::json PrintNode::Serialize() const {
    nlohmann::json j;
    j["id"] = id();
    j["kind"] = NodeKindToString(kind());
    j["name"] = name_;
    return j;
}

std::expected<void, std::string> PrintNode::Deserialize(
    const nlohmann::json &json) {
    if (json.contains("name") && json["name"].is_string()) {
        name_ = json["name"].get<std::string>();
    }
    return {};
}

}  // namespace core
