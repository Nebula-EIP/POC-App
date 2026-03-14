#pragma once

#include <any>
#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

class LiteralNode : public NodeBase {
   public:
    ~LiteralNode() = default;

    void set_name(const std::string &name);
    const std::string &name() const noexcept;

    /**
     * @brief Change the type of the node & it's pins connections
     *
     * @throws PinStillConnectedException if pins are already connected
     */
    void set_type(PinDataType type);
    PinDataType type() const noexcept;

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
    std::any data() const noexcept;

    uint8_t GetInputPinCount() const noexcept override;
    uint8_t GetOutputPinCount() const noexcept override;

    PinDataType GetInputPinType(uint8_t pin) const override;
    PinDataType GetOutputPinType(uint8_t pin) const override;

    std::expected<void, std::string> CanConnectTo(
        uint8_t out_pin, const NodeBase *target,
        uint8_t in_pin) const noexcept override;

    std::string GetInputPinName(uint8_t pin) const override;
    std::string GetOutputPinName(uint8_t pin) const override;

    std::string GetDisplayName() const noexcept override;
    std::string GetCategory() const noexcept override;

    nlohmann::json Serialize() const override;

    /**
     * @brief Deserializes this LiteralNode's data from JSON.
     * @param json The JSON object containing the literal node data.
     * @return An expected containing void on success, or an error message on
     *         failure.
     */
    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

   private:
    friend Graph;
    friend NodeBase;

    LiteralNode(uint32_t id, NodeKind kind) noexcept;

    void InitializeConnections() override;

   private:
    PinDataType type_ = PinDataType::kInt;
    std::string name_ = "Literal";
    std::any data_;
};

}  // namespace core
