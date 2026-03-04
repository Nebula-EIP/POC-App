#pragma once

#include <any>
#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

class VariableNode : public NodeBase {
   public:
    ~VariableNode() = default;

    void set_name(const std::string &name);
    const std::string &name() const;

    void set_type(PinDataType type);
    PinDataType type() const;

    /**
     * @brief Set the data to be stored in the node.
     *
     * It is the caller responsibility to store data that is the correct
     * type which can be retreived with LiteralNode::type().
     */
    void set_data(std::any data);

    /**
     * @brief Retrieves the data stored in the literal node.
     *
     * This function returns the data encapsulated within the literal node
     * as a std::any object. The actual type of the data can be determined
     * by the caller using std::any_cast along with LiteralNode::type().
     *
     * @return std::any The data stored in the literal node.
     */
    std::any data() const;

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
     * @brief Deserializes this VariableNode's data from JSON.
     * @param json The JSON object containing the variable node data.
     * @return An expected containing void on success, or an error message on
     *         failure.
     */
    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

   private:
    friend Graph;
    friend NodeBase;

    VariableNode(uint32_t id, NodeKind kind);

   private:
    PinDataType type_ = PinDataType::kUndefined;
    std::string name_ = "Literal";
    std::any data_;
};

}  // namespace core
