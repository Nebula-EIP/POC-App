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

    void SetName(const std::string &name);
    const std::string &Name() const noexcept;

    // -- Return type (single output) --

    void set_return_type(PinDataType type);
    PinDataType return_type() const noexcept;

    // -- Parameters (input pins) --

    /**
     * @brief Adds a parameter to the function.
     *
     * This increases the input pin count by one. The caller must call
     * ReinitializeConnections() afterwards if the node is already connected.
     *
     * @param name  The parameter name.
     * @param type  The parameter data type.
     */
    void AddParameter(const std::string &name, PinDataType type);

    /**
     * @brief Removes a parameter by index.
     * (the caller must have unlinked beforehand)
     * @param index The 0-based index of the parameter to remove.
     */
    void RemoveParameter(uint8_t index);

    /**
     * @brief Removes a parameter by name.
     * (the caller must have unlinked beforehand)
     * @param name The name of the parameter to remove.
     */
    void RemoveParameter(const std::string &name);

    /**
     * @brief Returns the parameter list.
     */
    const std::vector<FunctionParameter> &Parameters() const noexcept;

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

    FunctionNode(uint32_t id, NodeKind kind,
                 std::pair<float, float> postion) noexcept;

    void InitializeConnections() override;

   private:
    std::string name_ = "Function";
    PinDataType return_type_ = PinDataType::kVoid;
    std::vector<FunctionParameter> parameters_;
    std::unique_ptr<Graph> body_;
};

}  // namespace core
