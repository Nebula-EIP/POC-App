#pragma once

#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

class LoopNode : public NodeBase {
   public:
    ~LoopNode() = default;

    void SetName(const std::string &name);
    const std::string &Name() const noexcept;

    uint8_t GetInputPinCount() const noexcept;
    uint8_t GetOutputPinCount() const noexcept;

    PinDataType GetInputPinType(uint8_t pin) const;
    PinDataType GetOutputPinType(uint8_t pin) const;

    std::expected<void, std::string> CanConnectTo(
        uint8_t out_pin, const NodeBase *target,
        uint8_t in_pin) const noexcept override;

    std::string GetDisplayName() const noexcept override;
    std::string GetCategory() const noexcept override;

    nlohmann::json Serialize() const override;
    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

   private:
    friend Graph;
    friend NodeBase;

    LoopNode(uint32_t id, NodeKind kind,
             utils::WrappedVector2 position) noexcept;
    void InitializeConnections() override;

   private:
    std::string name_ = "While";
};

}  // namespace core
