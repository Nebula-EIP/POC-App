#pragma once

#include <any>
#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

/**
 * @brief Conditional branching node (if/else/then)
 */
class ConditionNode : public NodeBase {
   public:
    ~ConditionNode() = default;

    void set_name(const std::string &name);
    const std::string &name() const;

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

    /**
     * @brief Deserializes this ConditionNode's data from JSON.
     * @param json The JSON object containing the variable node data.
     * @return An expected containing void on success, or an error message on
     *         failure.
     */
    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

   private:
    friend Graph;
    friend NodeBase;

    ConditionNode(uint32_t id, NodeKind kind);

   private:
    std::string name_ = "If/Else/Then";
};

}  // namespace core
