#pragma once

#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

/**
 * @class FunctionInputNode
 * @brief A node representing a single function input parameter inside the
 *        function's body graph.
 *
 * Each FunctionInputNode has exactly one output pin whose data type
 * matches the corresponding function parameter.
 */
class FunctionInputNode : public NodeBase {
   public:
    ~FunctionInputNode() = default;

    void SetName(const std::string &name);
    const std::string &Name() const noexcept;

    /**
     * @brief Change the type of the node & it's pins connections
     *
     * @throws PinStillConnectedException if pins are already connected
     */
    void SetType(PinDataType type);
    PinDataType Type() const noexcept;

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

    FunctionInputNode(uint32_t id, NodeKind kind,
                      std::pair<float, float> position) noexcept;

    void InitializeConnections() override;

   private:
    PinDataType type_ = PinDataType::kUndefined;
    std::string name_ = "Input";
};

}  // namespace core
