#pragma once

#include "../node_base.hpp"

namespace core {

class Graph;

class LiteralNode : public NodeBase {
 public:
    ~LiteralNode() = default;

    uint8_t GetInputPinCount() const override;
    uint8_t GetOutputPinCount() const override;

    PinDataType GetInputPinType(uint8_t pin) const override;
    PinDataType GetOutputPinType(uint8_t pin) const override;

    std::expected<void, std::string> CanConnectTo(
        uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const override;

    std::string GetInputPinName(uint8_t pin) const override;
    std::string GetOutputPinName(uint8_t pin) const override;

    std::string GetDisplayName() const override;
    std::string GetCategory() const override;

 private:
    friend Graph;

    LiteralNode(uint32_t id, NodeKind kind);

 private:
    PinDataType type_ = PinDataType::kUndefined;
    std::string name_ = "Literal";
    
};

}  // namespace core
