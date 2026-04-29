#pragma once

#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

/**
 * @class FunctionOutputNode
 * @brief A node representing the return value inside the function's body graph.
 *
 * Each FunctionOutputNode has exactly one input pin whose data type
 * matches the function's return type.
 */
class FunctionOutputNode : public NodeBase {
   public:
    ~FunctionOutputNode() = default;

    void set_name(const std::string &name);
    const std::string &name() const noexcept;

    /**
     * @brief Change the type of the node & it's pins connections
     *
     * @throws PinStillConnectedException if pins are already connected
     */
    void set_type(PinDataType type);
    PinDataType type() const noexcept;

    // -- NodeBase overrides --

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

    FunctionOutputNode(uint32_t id, NodeKind kind, std::pair<float, float> postion) noexcept;

    void InitializeConnections() override;

   private:
    PinDataType type_ = PinDataType::kUndefined;
    std::string name_ = "Output";
};

}  // namespace core
