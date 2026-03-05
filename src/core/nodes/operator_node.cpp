#include "operator_node.hpp"

#include <stdexcept>

namespace core {

OperatorNode::OperatorNode(uint32_t id, NodeKind kind) : NodeBase(id, kind) {
    parents_.resize(GetInputPinCount());
    childrens_.resize(GetOutputPinCount());
}

void OperatorNode::set_operator_type(OperatorType type) {
    operator_type_ = type;
}

OperatorNode::OperatorType OperatorNode::operator_type() const {
    return operator_type_;
}

void OperatorNode::set_name(const std::string &name) { name_ = name; }

const std::string &OperatorNode::name() const { return name_; }

uint8_t OperatorNode::GetInputPinCount() const {
    return IsUnaryOperator() ? 1 : 2;
}

uint8_t OperatorNode::GetOutputPinCount() const { return 1; }

NodeBase::PinDataType OperatorNode::GetInputPinType(uint8_t pin) const {
    if (pin >= GetInputPinCount()) {
        return PinDataType::kUndefined;
    }
    return GetOperatorInputType();
}

NodeBase::PinDataType OperatorNode::GetOutputPinType(uint8_t pin) const {
    if (pin != 0) {
        return PinDataType::kUndefined;
    }
    return GetOperatorOutputType();
}

std::expected<void, std::string> OperatorNode::CanConnectTo(
    uint8_t out_pin, const NodeBase *target, uint8_t in_pin) const {
    if (out_pin != 0) {
        return std::unexpected("Output pin does not exist");
    }

    if (in_pin >= target->GetInputPinCount()) {
        return std::unexpected("Target input pin does not exist");
    }

    if (GetOutputPinType(out_pin) != target->GetInputPinType(in_pin)) {
        return std::unexpected("Type mismatch between output and target input");
    }

    return {};
}

std::string OperatorNode::GetInputPinName(uint8_t pin) const {
    if (IsUnaryOperator()) {
        if (pin == 0) return "Input";
    } else {
        if (pin == 0) return "A";
        if (pin == 1) return "B";
    }
    return "";
}

std::string OperatorNode::GetOutputPinName(uint8_t pin) const {
    if (pin == 0) {
        return "Result";
    }
    return "";
}

std::string OperatorNode::GetDisplayName() const { return name_; }

std::string OperatorNode::GetCategory() const {
    switch (operator_type_) {
        case OperatorType::kAddition:
        case OperatorType::kSubtraction:
        case OperatorType::kMultiplication:
        case OperatorType::kDivision:
        case OperatorType::kModulo:
            return "Arithmetic";

        case OperatorType::kBitwiseAnd:
        case OperatorType::kBitwiseOr:
        case OperatorType::kBitwiseXor:
        case OperatorType::kBitwiseNot:
        case OperatorType::kLeftShift:
        case OperatorType::kRightShift:
            return "Bitwise";

        case OperatorType::kEqual:
        case OperatorType::kNotEqual:
        case OperatorType::kLessThan:
        case OperatorType::kGreaterThan:
        case OperatorType::kLessOrEqual:
        case OperatorType::kGreaterOrEqual:
            return "Comparison";

        case OperatorType::kLogicalAnd:
        case OperatorType::kLogicalOr:
        case OperatorType::kLogicalNot:
            return "Logical";

        default:
            return "Operators";
    }
}

nlohmann::json OperatorNode::Serialize() const {
    nlohmann::json json;
    json["id"] = id();
    json["kind"] = NodeKindToString(kind());
    json["operator_type"] = OperatorTypeToString(operator_type_);
    json["name"] = name_;
    return json;
}

std::expected<void, std::string> OperatorNode::Deserialize(
    const nlohmann::json &json) {
    if (!json.contains("operator_type") || !json.contains("name")) {
        return std::unexpected(
            "Missing required fields for OperatorNode: operator_type or name");
    }

    try {
        std::string operator_type_str =
            json["operator_type"].get<std::string>();
        std::string name = json["name"].get<std::string>();

        operator_type_ = StringToOperatorType(operator_type_str);
        name_ = name;

        return {};
    } catch (const std::exception &e) {
        return std::unexpected(
            std::string("Failed to deserialize OperatorNode: ") + e.what());
    }
}

bool OperatorNode::IsUnaryOperator() const {
    return operator_type_ == OperatorType::kBitwiseNot ||
           operator_type_ == OperatorType::kLogicalNot;
}

NodeBase::PinDataType OperatorNode::GetOperatorInputType() const {
    switch (operator_type_) {
        // Arithmetic operators: Int or Float
        case OperatorType::kAddition:
        case OperatorType::kSubtraction:
        case OperatorType::kMultiplication:
        case OperatorType::kDivision:
            return PinDataType::kInt;  // Can be extended to support Float

        // Modulo: Int only
        case OperatorType::kModulo:
            return PinDataType::kInt;

        // Bitwise operators: Int only
        case OperatorType::kBitwiseAnd:
        case OperatorType::kBitwiseOr:
        case OperatorType::kBitwiseXor:
        case OperatorType::kBitwiseNot:
        case OperatorType::kLeftShift:
        case OperatorType::kRightShift:
            return PinDataType::kInt;

        // Comparison operators: Int or Float
        case OperatorType::kEqual:
        case OperatorType::kNotEqual:
        case OperatorType::kLessThan:
        case OperatorType::kGreaterThan:
        case OperatorType::kLessOrEqual:
        case OperatorType::kGreaterOrEqual:
            return PinDataType::kInt;  // Can be extended to support Float

        // Logical operators: Bool
        case OperatorType::kLogicalAnd:
        case OperatorType::kLogicalOr:
        case OperatorType::kLogicalNot:
            return PinDataType::kBool;

        default:
            return PinDataType::kUndefined;
    }
}

NodeBase::PinDataType OperatorNode::GetOperatorOutputType() const {
    switch (operator_type_) {
        // Arithmetic operators: Same as input type (Int or Float)
        case OperatorType::kAddition:
        case OperatorType::kSubtraction:
        case OperatorType::kMultiplication:
        case OperatorType::kDivision:
        case OperatorType::kModulo:
            return PinDataType::kInt;  // Same as input type

        // Bitwise operators: Int
        case OperatorType::kBitwiseAnd:
        case OperatorType::kBitwiseOr:
        case OperatorType::kBitwiseXor:
        case OperatorType::kBitwiseNot:
        case OperatorType::kLeftShift:
        case OperatorType::kRightShift:
            return PinDataType::kInt;

        // Comparison operators: Bool
        case OperatorType::kEqual:
        case OperatorType::kNotEqual:
        case OperatorType::kLessThan:
        case OperatorType::kGreaterThan:
        case OperatorType::kLessOrEqual:
        case OperatorType::kGreaterOrEqual:
            return PinDataType::kBool;

        // Logical operators: Bool
        case OperatorType::kLogicalAnd:
        case OperatorType::kLogicalOr:
        case OperatorType::kLogicalNot:
            return PinDataType::kBool;

        default:
            return PinDataType::kUndefined;
    }
}

std::string OperatorTypeToString(OperatorNode::OperatorType type) {
    switch (type) {
        case OperatorNode::OperatorType::kAddition:
            return "Addition";
        case OperatorNode::OperatorType::kSubtraction:
            return "Subtraction";
        case OperatorNode::OperatorType::kMultiplication:
            return "Multiplication";
        case OperatorNode::OperatorType::kDivision:
            return "Division";
        case OperatorNode::OperatorType::kModulo:
            return "Modulo";
        case OperatorNode::OperatorType::kBitwiseAnd:
            return "BitwiseAnd";
        case OperatorNode::OperatorType::kBitwiseOr:
            return "BitwiseOr";
        case OperatorNode::OperatorType::kBitwiseXor:
            return "BitwiseXor";
        case OperatorNode::OperatorType::kBitwiseNot:
            return "BitwiseNot";
        case OperatorNode::OperatorType::kLeftShift:
            return "LeftShift";
        case OperatorNode::OperatorType::kRightShift:
            return "RightShift";
        case OperatorNode::OperatorType::kEqual:
            return "Equal";
        case OperatorNode::OperatorType::kNotEqual:
            return "NotEqual";
        case OperatorNode::OperatorType::kLessThan:
            return "LessThan";
        case OperatorNode::OperatorType::kGreaterThan:
            return "GreaterThan";
        case OperatorNode::OperatorType::kLessOrEqual:
            return "LessOrEqual";
        case OperatorNode::OperatorType::kGreaterOrEqual:
            return "GreaterOrEqual";
        case OperatorNode::OperatorType::kLogicalAnd:
            return "LogicalAnd";
        case OperatorNode::OperatorType::kLogicalOr:
            return "LogicalOr";
        case OperatorNode::OperatorType::kLogicalNot:
            return "LogicalNot";
        default:
            return "Unknown";
    }
}

OperatorNode::OperatorType StringToOperatorType(const std::string &str) {
    if (str == "Addition") return OperatorNode::OperatorType::kAddition;
    if (str == "Subtraction") return OperatorNode::OperatorType::kSubtraction;
    if (str == "Multiplication") {
        return OperatorNode::OperatorType::kMultiplication;
    }
    if (str == "Division") return OperatorNode::OperatorType::kDivision;
    if (str == "Modulo") return OperatorNode::OperatorType::kModulo;
    if (str == "BitwiseAnd") return OperatorNode::OperatorType::kBitwiseAnd;
    if (str == "BitwiseOr") return OperatorNode::OperatorType::kBitwiseOr;
    if (str == "BitwiseXor") return OperatorNode::OperatorType::kBitwiseXor;
    if (str == "BitwiseNot") return OperatorNode::OperatorType::kBitwiseNot;
    if (str == "LeftShift") return OperatorNode::OperatorType::kLeftShift;
    if (str == "RightShift") return OperatorNode::OperatorType::kRightShift;
    if (str == "Equal") return OperatorNode::OperatorType::kEqual;
    if (str == "NotEqual") return OperatorNode::OperatorType::kNotEqual;
    if (str == "LessThan") return OperatorNode::OperatorType::kLessThan;
    if (str == "GreaterThan") return OperatorNode::OperatorType::kGreaterThan;
    if (str == "LessOrEqual") return OperatorNode::OperatorType::kLessOrEqual;
    if (str == "GreaterOrEqual") {
        return OperatorNode::OperatorType::kGreaterOrEqual;
    }
    if (str == "LogicalAnd") return OperatorNode::OperatorType::kLogicalAnd;
    if (str == "LogicalOr") return OperatorNode::OperatorType::kLogicalOr;
    if (str == "LogicalNot") return OperatorNode::OperatorType::kLogicalNot;

    throw std::invalid_argument("Unknown operator type: " + str);
}

}  // namespace core
