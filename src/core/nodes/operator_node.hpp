#pragma once

#include <string>

#include "../node_base.hpp"

namespace core {

class Graph;

/**
 * @class OperatorNode
 * @brief Represents all binary operators (arithmetic, logical, bitwise,
 * comparison)
 *
 * The OperatorNode class handles all types of binary operations that take
 * two inputs and produce one output.
 */
class OperatorNode : public NodeBase {
   public:
    /**
     * @enum OperatorType
     * @brief Defines all supported operator types
     */
    enum class OperatorType {
        // Arithmetic operators
        kAddition,        ///< a + b
        kSubtraction,     ///< a - b
        kMultiplication,  ///< a * b
        kDivision,        ///< a / b
        kModulo,          ///< a % b

        // Bitwise operators
        kBitwiseAnd,  ///< a & b
        kBitwiseOr,   ///< a | b
        kBitwiseXor,  ///< a ^ b
        kBitwiseNot,  ///< ~a (unary, but uses second input)
        kLeftShift,   ///< a << b
        kRightShift,  ///< a >> b

        // Comparison operators
        kEqual,           ///< a == b
        kNotEqual,        ///< a != b
        kLessThan,        ///< a < b
        kGreaterThan,     ///< a > b
        kLessOrEqual,     ///< a <= b
        kGreaterOrEqual,  ///< a >= b

        // Logical operators
        kLogicalAnd,  ///< a && b
        kLogicalOr,   ///< a || b
        kLogicalNot   ///< !a (unary, but uses first input)
    };

    ~OperatorNode() = default;

    void set_operator_type(OperatorType type);
    OperatorType operator_type() const;

    void set_name(const std::string &name);
    const std::string &name() const;

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
     * @brief Deserializes this OperatorNode's data from JSON.
     * @param json The JSON object containing the operator node data.
     * @return An expected containing void on success, or an error message on
     *         failure.
     */
    std::expected<void, std::string> Deserialize(
        const nlohmann::json &json) override;

    // Helper functions
    /**
     * @brief Determines if an operator is unary (takes one input)
     * @return true if the operator is unary, false otherwise
     */
    bool IsUnaryOperator() const;

    /**
     * @brief Get the input data type for this operator
     * @return The data type that inputs should have
     */
    PinDataType GetOperatorInputType() const;

    /**
     * @brief Get the output data type for this operator
     * @return The data type that the operator produces
     */
    PinDataType GetOperatorOutputType() const;

   private:
    friend Graph;
    friend NodeBase;

    OperatorNode(uint32_t id, NodeKind kind, std::pair<float, float> position) noexcept;

    void InitializeConnections() override;

   private:
    OperatorType operator_type_ = OperatorType::kAddition;
    std::string name_ = "Operator";
};

std::string OperatorTypeToString(OperatorNode::OperatorType type);
OperatorNode::OperatorType StringToOperatorType(const std::string &str);

}  // namespace core
