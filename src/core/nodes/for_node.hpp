#pragma once

#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

class ForNode : public NodeBase {
   public:
    ~ForNode() = default;

    /**
     * @brief Sets the display name of the for loop node.
     * @param name The new name to set for the for loop node.
     */
    void SetName(const std::string &name);

    /**
     * @brief Gets the display name of the for loop node.
     * @return The current name of the for loop node.
     */
    const std::string &Name() const noexcept;

    /**
     * @brief Gets the number of input pins for this node.
     * @return The count of input pins.
     */
    uint8_t GetInputPinCount() const noexcept;

    /**
     * @brief Gets the number of output pins for this node.
     * @return The count of output pins.
     */
    uint8_t GetOutputPinCount() const noexcept;

    /**
     * @brief Gets the data type of a specific input pin.
     * @param pin The index of the input pin.
     * @return The data type of the specified input pin.
     */
    PinDataType GetInputPinType(uint8_t pin) const;

    /**
     * @brief Gets the data type of a specific output pin.
     * @param pin The index of the output pin.
     * @return The data type of the specified output pin.
     */
    PinDataType GetOutputPinType(uint8_t pin) const;

    /**
     * @brief Determines if this node can connect to a target node's input pin.
     * @param out_pin The index of the output pin on this node.
     * @param target The target node to connect to.
     * @param in_pin The index of the input pin on the target node.
     * @return An expected value indicating success or an error message.
     */
    std::expected<void, std::string> CanConnectTo(
        uint8_t out_pin, const NodeBase *target,
        uint8_t in_pin) const noexcept override;

    /**
     * @brief Gets the display name of this node for UI purposes.
     * @return The display name of the node.
     */
    std::string GetDisplayName() const noexcept override;

    /**
     * @brief Gets the category of this node for organizational purposes in the
     * UI.
     * @return The category name of the node.
     */
    std::string GetCategory() const noexcept override;

    /**
     * @brief Serializes the node to JSON format.
     * @return JSON object containing the serialized node data.
     */
    nlohmann::json Serialize() const override;

    /**
     * @brief Deserializes the node from JSON format.
     * @param json JSON object containing the serialized node data.
     * @return An expected value indicating success or an error message.
     */
    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

   private:
    friend Graph;
    friend NodeBase;

    /**
     * @brief Private constructor for ForNode, called by the Graph when creating
     * a new node.
     * @param id The unique identifier for this node.
     * @param kind The kind of the node, should be NodeKind::kFor.
     * @param position The initial position of the node in the editor.
     */
    ForNode(uint32_t id, NodeKind kind,
            utils::WrappedVector2 position) noexcept;

    /**
     * @brief Initializes the connections for this node.
     */
    void InitializeConnections() override;

   private:
    std::string name_ = "i";
};

}  // namespace core
