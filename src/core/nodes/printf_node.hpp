#pragma once

#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

class PrintfNode : public NodeBase {
   public:
    ~PrintfNode() = default;

    void set_type(PinDataType type);
    PinDataType type() const;

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

    nlohmann::json Serialize() const override;

    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

   private:
    friend Graph;
    friend NodeBase;

    PrintfNode(uint32_t id, NodeKind kind);

   private:
    PinDataType type_ = PinDataType::kString;
};

}  // namespace core
