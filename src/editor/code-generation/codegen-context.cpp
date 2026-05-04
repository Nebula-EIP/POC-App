#include "codegen-context.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

#include "core/graph.hpp"
#include "core/node_base.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/operator_node.hpp"

namespace {

using PinDataType = core::NodeBase::PinDataType;
using OperatorType = core::OperatorNode::OperatorType;

using ConstantScalar = std::variant<long long, double, bool, std::string>;

struct ConstantValue {
    PinDataType type = PinDataType::kUndefined;
    ConstantScalar value{};
};

static std::string EscapeString(const std::string &value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}

static std::string LiteralToCpp(const core::LiteralNode &lit) {
    std::ostringstream oss;
    switch (lit.type()) {
        case PinDataType::kInt: {
            try {
                oss << std::any_cast<int>(lit.data());
            } catch (...) {
                oss << 0;
            }
            break;
        }
        case PinDataType::kFloat: {
            try {
                oss << std::any_cast<double>(lit.data());
            } catch (...) {
                oss << 0.0;
            }
            break;
        }
        case PinDataType::kBool: {
            try {
                oss << (std::any_cast<bool>(lit.data()) ? "true" : "false");
            } catch (...) {
                oss << "false";
            }
            break;
        }
        case PinDataType::kString: {
            try {
                oss << '"' << EscapeString(std::any_cast<std::string>(lit.data())) << '"';
            } catch (...) {
                oss << "\"\"";
            }
            break;
        }
        default:
            oss << 0;
            break;
    }
    return oss.str();
}

static std::string ConstantToCpp(const ConstantValue &value) {
    std::ostringstream oss;
    switch (value.type) {
        case PinDataType::kInt:
            oss << std::get<long long>(value.value);
            break;
        case PinDataType::kFloat:
            oss << std::get<double>(value.value);
            break;
        case PinDataType::kBool:
            oss << (std::get<bool>(value.value) ? "true" : "false");
            break;
        case PinDataType::kString:
            oss << '"' << EscapeString(std::get<std::string>(value.value)) << '"';
            break;
        default:
            oss << 0;
            break;
    }
    return oss.str();
}

static std::string OpSymbol(OperatorType type) {
    switch (type) {
        case OperatorType::kAddition: return "+";
        case OperatorType::kSubtraction: return "-";
        case OperatorType::kMultiplication: return "*";
        case OperatorType::kDivision: return "/";
        case OperatorType::kModulo: return "%";
        case OperatorType::kBitwiseAnd: return "&";
        case OperatorType::kBitwiseOr: return "|";
        case OperatorType::kBitwiseXor: return "^";
        case OperatorType::kLeftShift: return "<<";
        case OperatorType::kRightShift: return ">>";
        case OperatorType::kEqual: return "==";
        case OperatorType::kNotEqual: return "!=";
        case OperatorType::kLessThan: return "<";
        case OperatorType::kGreaterThan: return ">";
        case OperatorType::kLessOrEqual: return "<=";
        case OperatorType::kGreaterOrEqual: return ">=";
        case OperatorType::kLogicalAnd: return "&&";
        case OperatorType::kLogicalOr: return "||";
        default: return "+";
    }
}

static std::string CppTypeFor(PinDataType type) {
    switch (type) {
        case PinDataType::kInt: return "int";
        case PinDataType::kFloat: return "double";
        case PinDataType::kBool: return "bool";
        case PinDataType::kString: return "std::string";
        case PinDataType::kVoid:
        case PinDataType::kUndefined:
        default: return "auto";
    }
}

static std::string DefaultCppExprFor(PinDataType type) {
    switch (type) {
        case PinDataType::kInt: return "0";
        case PinDataType::kFloat: return "0.0";
        case PinDataType::kBool: return "false";
        case PinDataType::kString: return "std::string{}";
        case PinDataType::kVoid:
        case PinDataType::kUndefined:
        default: return "0";
    }
}

static std::optional<ConstantValue> GetLiteralConstant(const core::LiteralNode &lit) {
    try {
        switch (lit.type()) {
            case PinDataType::kInt:
                return ConstantValue{PinDataType::kInt, static_cast<long long>(std::any_cast<int>(lit.data()))};
            case PinDataType::kFloat:
                return ConstantValue{PinDataType::kFloat, std::any_cast<double>(lit.data())};
            case PinDataType::kBool:
                return ConstantValue{PinDataType::kBool, std::any_cast<bool>(lit.data())};
            case PinDataType::kString:
                return ConstantValue{PinDataType::kString, std::any_cast<std::string>(lit.data())};
            default:
                return std::nullopt;
        }
    } catch (...) {
        return std::nullopt;
    }
}

static bool IsNumeric(PinDataType type) {
    return type == PinDataType::kInt || type == PinDataType::kFloat;
}

static double AsDouble(const ConstantValue &value) {
    if (value.type == PinDataType::kInt) {
        return static_cast<double>(std::get<long long>(value.value));
    }
    return std::get<double>(value.value);
}

static long long AsInt(const ConstantValue &value) {
    if (value.type == PinDataType::kInt) {
        return std::get<long long>(value.value);
    }
    return static_cast<long long>(std::get<double>(value.value));
}

static bool ToBool(const ConstantValue &value) {
    switch (value.type) {
        case PinDataType::kBool: return std::get<bool>(value.value);
        case PinDataType::kInt: return std::get<long long>(value.value) != 0;
        case PinDataType::kFloat: return std::get<double>(value.value) != 0.0;
        default: return false;
    }
}

static std::optional<ConstantValue> FoldUnary(OperatorType type,
                                              const ConstantValue &value) {
    switch (type) {
        case OperatorType::kBitwiseNot:
            if (value.type == PinDataType::kInt) {
                return ConstantValue{PinDataType::kInt, ~AsInt(value)};
            }
            return std::nullopt;
        case OperatorType::kLogicalNot:
            if (value.type == PinDataType::kBool || IsNumeric(value.type)) {
                return ConstantValue{PinDataType::kBool, !ToBool(value)};
            }
            return std::nullopt;
        default:
            return std::nullopt;
    }
}

static std::optional<ConstantValue> FoldBinary(OperatorType type,
                                               const ConstantValue &left,
                                               const ConstantValue &right) {
    switch (type) {
        case OperatorType::kAddition:
        case OperatorType::kSubtraction:
        case OperatorType::kMultiplication:
        case OperatorType::kDivision: {
            if (!IsNumeric(left.type) || !IsNumeric(right.type)) {
                return std::nullopt;
            }

            const bool use_float = left.type == PinDataType::kFloat || right.type == PinDataType::kFloat;
            if (use_float) {
                const double lhs = AsDouble(left);
                const double rhs = AsDouble(right);
                switch (type) {
                    case OperatorType::kAddition:
                        return ConstantValue{PinDataType::kFloat, lhs + rhs};
                    case OperatorType::kSubtraction:
                        return ConstantValue{PinDataType::kFloat, lhs - rhs};
                    case OperatorType::kMultiplication:
                        return ConstantValue{PinDataType::kFloat, lhs * rhs};
                    case OperatorType::kDivision:
                        if (rhs == 0.0) return std::nullopt;
                        return ConstantValue{PinDataType::kFloat, lhs / rhs};
                    default:
                        return std::nullopt;
                }
            }

            const long long lhs = AsInt(left);
            const long long rhs = AsInt(right);
            switch (type) {
                case OperatorType::kAddition:
                    return ConstantValue{PinDataType::kInt, lhs + rhs};
                case OperatorType::kSubtraction:
                    return ConstantValue{PinDataType::kInt, lhs - rhs};
                case OperatorType::kMultiplication:
                    return ConstantValue{PinDataType::kInt, lhs * rhs};
                case OperatorType::kDivision:
                    if (rhs == 0) return std::nullopt;
                    return ConstantValue{PinDataType::kInt, lhs / rhs};
                default:
                    return std::nullopt;
            }
        }
        case OperatorType::kModulo:
            if (left.type == PinDataType::kInt && right.type == PinDataType::kInt) {
                const long long rhs = AsInt(right);
                if (rhs == 0) return std::nullopt;
                return ConstantValue{PinDataType::kInt, AsInt(left) % rhs};
            }
            return std::nullopt;
        case OperatorType::kBitwiseAnd:
        case OperatorType::kBitwiseOr:
        case OperatorType::kBitwiseXor:
        case OperatorType::kLeftShift:
        case OperatorType::kRightShift:
            if (left.type == PinDataType::kInt && right.type == PinDataType::kInt) {
                const long long lhs = AsInt(left);
                const long long rhs = AsInt(right);
                switch (type) {
                    case OperatorType::kBitwiseAnd:
                        return ConstantValue{PinDataType::kInt, lhs & rhs};
                    case OperatorType::kBitwiseOr:
                        return ConstantValue{PinDataType::kInt, lhs | rhs};
                    case OperatorType::kBitwiseXor:
                        return ConstantValue{PinDataType::kInt, lhs ^ rhs};
                    case OperatorType::kLeftShift:
                        return ConstantValue{PinDataType::kInt, lhs << rhs};
                    case OperatorType::kRightShift:
                        return ConstantValue{PinDataType::kInt, lhs >> rhs};
                    default:
                        return std::nullopt;
                }
            }
            return std::nullopt;
        case OperatorType::kEqual:
        case OperatorType::kNotEqual:
        case OperatorType::kLessThan:
        case OperatorType::kGreaterThan:
        case OperatorType::kLessOrEqual:
        case OperatorType::kGreaterOrEqual: {
            if (left.type != right.type && !(IsNumeric(left.type) && IsNumeric(right.type))) {
                return std::nullopt;
            }

            bool result = false;
            if (left.type == PinDataType::kString && right.type == PinDataType::kString) {
                const auto &lhs = std::get<std::string>(left.value);
                const auto &rhs = std::get<std::string>(right.value);
                if (type == OperatorType::kEqual) result = lhs == rhs;
                else if (type == OperatorType::kNotEqual) result = lhs != rhs;
                else return std::nullopt;
            } else if (left.type == PinDataType::kBool && right.type == PinDataType::kBool) {
                const bool lhs = std::get<bool>(left.value);
                const bool rhs = std::get<bool>(right.value);
                switch (type) {
                    case OperatorType::kEqual: result = lhs == rhs; break;
                    case OperatorType::kNotEqual: result = lhs != rhs; break;
                    case OperatorType::kLessThan: result = lhs < rhs; break;
                    case OperatorType::kGreaterThan: result = lhs > rhs; break;
                    case OperatorType::kLessOrEqual: result = lhs <= rhs; break;
                    case OperatorType::kGreaterOrEqual: result = lhs >= rhs; break;
                    default: return std::nullopt;
                }
            } else {
                const bool use_float = left.type == PinDataType::kFloat || right.type == PinDataType::kFloat;
                if (use_float) {
                    const double lhs = AsDouble(left);
                    const double rhs = AsDouble(right);
                    switch (type) {
                        case OperatorType::kEqual: result = lhs == rhs; break;
                        case OperatorType::kNotEqual: result = lhs != rhs; break;
                        case OperatorType::kLessThan: result = lhs < rhs; break;
                        case OperatorType::kGreaterThan: result = lhs > rhs; break;
                        case OperatorType::kLessOrEqual: result = lhs <= rhs; break;
                        case OperatorType::kGreaterOrEqual: result = lhs >= rhs; break;
                        default: return std::nullopt;
                    }
                } else {
                    const long long lhs = AsInt(left);
                    const long long rhs = AsInt(right);
                    switch (type) {
                        case OperatorType::kEqual: result = lhs == rhs; break;
                        case OperatorType::kNotEqual: result = lhs != rhs; break;
                        case OperatorType::kLessThan: result = lhs < rhs; break;
                        case OperatorType::kGreaterThan: result = lhs > rhs; break;
                        case OperatorType::kLessOrEqual: result = lhs <= rhs; break;
                        case OperatorType::kGreaterOrEqual: result = lhs >= rhs; break;
                        default: return std::nullopt;
                    }
                }
            }

            return ConstantValue{PinDataType::kBool, result};
        }
        case OperatorType::kLogicalAnd:
        case OperatorType::kLogicalOr:
            if ((left.type == PinDataType::kBool || IsNumeric(left.type)) &&
                (right.type == PinDataType::kBool || IsNumeric(right.type))) {
                const bool lhs = ToBool(left);
                const bool rhs = ToBool(right);
                if (type == OperatorType::kLogicalAnd) {
                    return ConstantValue{PinDataType::kBool, lhs && rhs};
                }
                return ConstantValue{PinDataType::kBool, lhs || rhs};
            }
            return std::nullopt;
        default:
            return std::nullopt;
    }
}

static const core::NodeBase::Connection *FindParentConnection(const core::NodeBase &node,
                                                              uint8_t pin) {
    const auto &parents = node.GetAllParents();
    const auto it = std::find_if(parents.begin(), parents.end(),
                                 [pin](const core::NodeBase::Connection &conn) {
                                     return conn.in_pin == pin;
                                 });
    if (it == parents.end()) {
        return nullptr;
    }
    return &(*it);
}

}  // namespace

::code_generation::CodeGeneratorFile editor::code_generation::CodegenContext::Generate(
    const core::Graph &graph) {
    ::code_generation::CodeGeneratorFile file;

    file.Line("#include <iostream>");
    file.Line("#include <string>");
    file.Line("");
    file.Line("int main() {");

    std::vector<const core::NodeBase *> nodes;
    nodes.reserve(graph.GetAllNodes().size());
    for (const auto &node_ptr : graph.GetAllNodes()) {
        nodes.push_back(node_ptr.get());
    }

    std::unordered_map<uint32_t, int> indegree;
    for (const auto *node : nodes) {
        indegree[node->id()] = 0;
    }

    for (const auto *node : nodes) {
        for (const auto &parent : node->GetAllParents()) {
            if (parent.IsConnected() && parent.node != nullptr) {
                ++indegree[node->id()];
            }
        }
    }

    std::queue<const core::NodeBase *> ready;
    for (const auto *node : nodes) {
        if (indegree[node->id()] == 0) {
            ready.push(node);
        }
    }

    std::vector<const core::NodeBase *> order;
    while (!ready.empty()) {
        const core::NodeBase *node = ready.front();
        ready.pop();
        order.push_back(node);

        for (const auto &child : node->GetAllChildrens()) {
            if (!child.IsConnected() || child.node == nullptr) {
                continue;
            }
            auto &current = indegree[child.node->id()];
            --current;
            if (current == 0) {
                ready.push(child.node);
            }
        }
    }

    if (order.size() < nodes.size()) {
        for (const auto *node : nodes) {
            if (std::find(order.begin(), order.end(), node) == order.end()) {
                order.push_back(node);
            }
        }
    }

    std::unordered_map<uint32_t, std::string> symbol_for_node;
    std::unordered_map<uint32_t, ConstantValue> folded_value_for_node;

    auto GetOperandExpr = [&](const core::NodeBase *node, uint8_t pin) -> std::string {
        const auto *connection = FindParentConnection(*node, pin);
        if (connection == nullptr || !connection->IsConnected() || connection->node == nullptr) {
            return DefaultCppExprFor(node->GetInputPinType(pin));
        }

        const auto it = symbol_for_node.find(connection->node->id());
        if (it != symbol_for_node.end()) {
            return it->second;
        }

        return DefaultCppExprFor(node->GetInputPinType(pin));
    };

    for (const auto *node : order) {
        switch (node->kind()) {
            case core::NodeBase::NodeKind::kLiteral: {
                const auto *literal = static_cast<const core::LiteralNode *>(node);
                std::ostringstream line;
                line << "const " << CppTypeFor(literal->type()) << " lit_" << literal->id()
                     << " = " << LiteralToCpp(*literal) << ";";
                file.Line("    " + line.str());

                symbol_for_node[node->id()] = "lit_" + std::to_string(node->id());
                if (auto constant = GetLiteralConstant(*literal)) {
                    folded_value_for_node[node->id()] = *constant;
                }
                break;
            }
            case core::NodeBase::NodeKind::kOperator: {
                const auto *op = static_cast<const core::OperatorNode *>(node);
                const PinDataType output_type = op->GetOutputPinType(0);
                const std::string symbol = "tmp_" + std::to_string(op->id());

                std::optional<ConstantValue> folded_value;
                if (op->IsUnaryOperator()) {
                    const auto *parent = FindParentConnection(*op, 0);
                    if (parent != nullptr && parent->IsConnected() && parent->node != nullptr) {
                        const auto it = folded_value_for_node.find(parent->node->id());
                        if (it != folded_value_for_node.end()) {
                            folded_value = FoldUnary(op->operator_type(), it->second);
                        }
                    }
                } else {
                    const auto *left_parent = FindParentConnection(*op, 0);
                    const auto *right_parent = FindParentConnection(*op, 1);
                    if (left_parent != nullptr && right_parent != nullptr &&
                        left_parent->IsConnected() && right_parent->IsConnected() &&
                        left_parent->node != nullptr && right_parent->node != nullptr) {
                        const auto left_it = folded_value_for_node.find(left_parent->node->id());
                        const auto right_it = folded_value_for_node.find(right_parent->node->id());
                        if (left_it != folded_value_for_node.end() &&
                            right_it != folded_value_for_node.end()) {
                            folded_value = FoldBinary(op->operator_type(), left_it->second,
                                                     right_it->second);
                        }
                    }
                }

                if (folded_value.has_value()) {
                    std::ostringstream line;
                    line << "const " << CppTypeFor(output_type) << " " << symbol
                         << " = " << ConstantToCpp(*folded_value) << ";";
                    file.Line("    " + line.str());
                    folded_value_for_node[node->id()] = *folded_value;
                    symbol_for_node[node->id()] = symbol;
                    break;
                }

                if (op->IsUnaryOperator()) {
                    const std::string operand = GetOperandExpr(op, 0);
                    std::ostringstream line;
                    line << CppTypeFor(output_type) << " " << symbol << " = "
                         << OpSymbol(op->operator_type()) << operand << ";";
                    file.Line("    " + line.str());
                } else {
                    const std::string left = GetOperandExpr(op, 0);
                    const std::string right = GetOperandExpr(op, 1);
                    std::ostringstream line;
                    line << CppTypeFor(output_type) << " " << symbol << " = " << left
                         << " " << OpSymbol(op->operator_type()) << " " << right << ";";
                    file.Line("    " + line.str());
                }

                symbol_for_node[node->id()] = symbol;
                break;
            }
            default: {
                symbol_for_node[node->id()] = "v" + std::to_string(node->id());
                break;
            }
        }
    }

    file.Line("");
    file.Line("    return 0;");
    file.Line("}");

    return file;
}
