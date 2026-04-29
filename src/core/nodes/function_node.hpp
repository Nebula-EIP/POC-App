#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../node_base.hpp"
#include "function_node_exceptions.hpp"

namespace core {

class Graph;

/**
 * @brief Represents a single function parameter (input pin).
 */
struct FunctionParameter {
    std::string name;
    NodeBase::PinDataType type = NodeBase::PinDataType::kUndefined;
    uint8_t pin_id = 0;
    uint32_t node_id = 0;
};

/**
 * @class FunctionNode
 * @brief A node that represents a user-defined function.
 *
 * A FunctionNode contains an inner Graph that holds the function body.
 * The number of input pins corresponds to the function parameters, and
 * there is exactly one output pin for the return value.
 */
class FunctionNode : public NodeBase {
   public:
    ~FunctionNode();

    // -- Name --

    void set_name(const std::string &name);
    const std::string &name() const noexcept;

    // -- Return type (single output) --

    void set_return_type(PinDataType type);
    PinDataType return_type() const noexcept;

    // -- Parameters (input pins) --

    const std::vector<FunctionParameter> &parameters() const noexcept;

    // -- Inner graph (function body) --

    /**
     * @brief Returns a mutable reference to the inner graph.
     */
    Graph &body();

    /**
     * @brief Returns a const reference to the inner graph.
     */
    const Graph &body() const;

    // -- NodeBase overrides --

    std::expected<void, std::string> CanConnectTo(
        uint8_t out_pin, const NodeBase *target,
        uint8_t in_pin) const noexcept override;

    std::string GetDisplayName() const noexcept override;
    std::string GetCategory() const noexcept override;

    nlohmann::json Serialize() const override;

    /**
     * @brief Deserializes this FunctionNode's data from JSON.
     * @param json The JSON object containing the function node data.
     * @return An expected containing void on success, or an error message on
     *         failure.
     */
    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

   private:
    friend Graph;
    friend NodeBase;

    FunctionNode(uint32_t id, NodeKind kind, std::pair<float, float> postion) noexcept;

    void InitializeConnections() override;

   private:
    std::string name_ = "Function";
    PinDataType return_type_ = PinDataType::kVoid;
    std::vector<FunctionParameter> parameters_;
    std::unique_ptr<Graph> body_;
};

}  // namespace core
