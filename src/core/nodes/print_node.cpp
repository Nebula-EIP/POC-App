#include "print_node.hpp"

#include <expected>

#include "../graph.hpp"

namespace core {

PrintNode::PrintNode(uint32_t id, NodeKind kind, utils::WrappedVector2 position) noexcept
    : NodeBase(id, kind, position) {
    InitializeConnections();
}

void PrintNode::InitializeConnections() {
    // Single input pin of string type for simplicity
    AddInputPin("value", PinDataType::kString);
}

void PrintNode::SetName(const std::string &name) { name_ = name; }
const std::string &PrintNode::Name() const noexcept { return name_; }

uint8_t PrintNode::GetInputPinCount() const noexcept { return 1; }

NodeBase::PinDataType PrintNode::GetInputPinType(uint8_t pin) const { return PinDataType::kString; }

std::expected<void, std::string> PrintNode::CanConnectTo(uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const noexcept {
    // Allow only string connections for now
    if (!target) {
        return std::unexpected(std::string("Target node is null"));
    }
    if (GetInputPinType(in_pin) != target->GetOutputPinType(out_pin)) {
        return std::unexpected(std::string("Incompatible pin types for PrintNode"));
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

std::expected<void, std::string> PrintNode::Deserialize(const nlohmann::json &json) {
    if (json.contains("name") && json["name"].is_string()) {
        name_ = json["name"].get<std::string>();
    }
    return {};
}

} // namespace core
