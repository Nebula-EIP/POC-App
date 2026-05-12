#include "codegen-context.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

#include "codegen-optimizer.hpp"
#include "core/graph.hpp"
#include "core/graph_validator.hpp"
#include "core/node_base.hpp"
#include "core/nodes/literal_node.hpp"
#include "core/nodes/operator_node.hpp"
#include "core/topological_sorter.hpp"

namespace {

using PinDataType = core::NodeBase::PinDataType;
using OperatorType = core::OperatorNode::OperatorType;

using ConstantScalar = std::variant<int64_t, double, bool, std::string>;

struct ConstantValue {
    PinDataType type_ = PinDataType::kUndefined;
    ConstantScalar value_{};\
};

static std::string EscapeString(const std::string &value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped.push_back(ch);
                break;
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
                oss << '"'
                    << EscapeString(std::any_cast<std::string>(lit.data()))
                    << '"';
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
    switch (value.type_) {
        case PinDataType::kInt:
            oss << std::get<int64_t>(value.value_);
            break;
        case PinDataType::kFloat:
            oss << std::get<double>(value.value_);
            break;
        case PinDataType::kBool:
            oss << (std::get<bool>(value.value_) ? "true" : "false");
            break;
        case PinDataType::kString:
            oss << '"' << EscapeString(std::get<std::string>(value.value_))
                << '"';
            break;
        default:
            oss << 0;
            break;
    }
    return oss.str();
}

static std::string OpSymbol(OperatorType type) {
    switch (type) {
        case OperatorType::kAddition:
            return "+";
        case OperatorType::kSubtraction:
            return "-";
        case OperatorType::kMultiplication:
            return "*";
        case OperatorType::kDivision:
            return "/";
        case OperatorType::kModulo:
            return "%";
        case OperatorType::kBitwiseAnd:
            return "&";
        case OperatorType::kBitwiseOr:
            return "|";
        case OperatorType::kBitwiseXor:
            return "^";
        case OperatorType::kLeftShift:
            return "<<";
        case OperatorType::kRightShift:
            return ">>";
        case OperatorType::kEqual:
            return "==";
        case OperatorType::kNotEqual:
            return "!=";
        case OperatorType::kLessThan:
            return "<";
        case OperatorType::kGreaterThan:
            return ">";
        case OperatorType::kLessOrEqual:
            return "<=";
        case OperatorType::kGreaterOrEqual:
            return ">=";
        case OperatorType::kLogicalAnd:
            return "&&";
        case OperatorType::kLogicalOr:
            return "||";
        default:
            return "+";
    }
}

static std::string CppTypeFor(PinDataType type) {
    switch (type) {
        case PinDataType::kInt:
            return "int";
        case PinDataType::kFloat:
            return "double";
        case PinDataType::kBool:
            return "bool";
        case PinDataType::kString:
            return "std::string";
        case PinDataType::kVoid:
        case PinDataType::kUndefined:
        default:
            return "auto";
    }
}

static std::string DefaultCppExprFor(PinDataType type) {
    switch (type) {
        case PinDataType::kInt:
            return "0";
        case PinDataType::kFloat:
            return "0.0";
        case PinDataType::kBool:
            return "false";
        case PinDataType::kString:
            return "std::string{}";
        case PinDataType::kVoid:
        case PinDataType::kUndefined:
        default:
            return "0";
    }
}

static std::optional<ConstantValue> GetLiteralConstant(
    const core::LiteralNode &lit) {
    try {
        switch (lit.type()) {
            case PinDataType::kInt:
                return ConstantValue{
                    PinDataType::kInt,
                    static_cast<int64_t>(std::any_cast<int>(lit.data()))};
            case PinDataType::kFloat:
                return ConstantValue{PinDataType::kFloat,
                                     std::any_cast<double>(lit.data())};
            case PinDataType::kBool:
                return ConstantValue{PinDataType::kBool,
                                     std::any_cast<bool>(lit.data())};
            case PinDataType::kString:
                return ConstantValue{PinDataType::kString,
                                     std::any_cast<std::string>(lit.data())};
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
    if (value.type_ == PinDataType::kInt) {
        return static_cast<double>(std::get<int64_t>(value.value_));
    }
    return std::get<double>(value.value_);
}

static int64_t AsInt(const ConstantValue &value) {
    if (value.type_ == PinDataType::kInt) {
        return std::get<int64_t>(value.value_);
    }
    return static_cast<int64_t>(std::get<double>(value.value_));
}

static bool ToBool(const ConstantValue &value) {
    switch (value.type_) {
        case PinDataType::kBool:
            return std::get<bool>(value.value_);
        case PinDataType::kInt:
            return std::get<int64_t>(value.value_) != 0;
        case PinDataType::kFloat:
            return std::get<double>(value.value_) != 0.0;
        default:
            return false;
    }
}

static std::optional<ConstantValue> FoldUnary(OperatorType type,
                                              const ConstantValue &value) {
    switch (type) {
        case OperatorType::kBitwiseNot:
            if (value.type_ == PinDataType::kInt) {
                return ConstantValue{PinDataType::kInt, ~AsInt(value)};
            }
            return std::nullopt;
        case OperatorType::kLogicalNot:
            if (value.type_ == PinDataType::kBool || IsNumeric(value.type_)) {
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
            if (!IsNumeric(left.type_) || !IsNumeric(right.type_)) {
                return std::nullopt;
            }

            const bool kUseFloat = left.type_ == PinDataType::kFloat ||
                                   right.type_ == PinDataType::kFloat;
            if (kUseFloat) {
                const double kLhs = AsDouble(left);
                const double kRhs = AsDouble(right);
                switch (type) {
                    case OperatorType::kAddition:
                        return ConstantValue{PinDataType::kFloat, kLhs + kRhs};
                    case OperatorType::kSubtraction:
                        return ConstantValue{PinDataType::kFloat, kLhs - kRhs};
                    case OperatorType::kMultiplication:
                        return ConstantValue{PinDataType::kFloat, kLhs * kRhs};
                    case OperatorType::kDivision:
                        if (kRhs == 0.0) return std::nullopt;
                        return ConstantValue{PinDataType::kFloat, kLhs / kRhs};
                    default:
                        return std::nullopt;
                }
            }

            const int64_t kLhs = AsInt(left);
            const int64_t kRhs = AsInt(right);
            switch (type) {
                case OperatorType::kAddition:
                    return ConstantValue{PinDataType::kInt, kLhs + kRhs};
                case OperatorType::kSubtraction:
                    return ConstantValue{PinDataType::kInt, kLhs - kRhs};
                case OperatorType::kMultiplication:
                    return ConstantValue{PinDataType::kInt, kLhs * kRhs};
                case OperatorType::kDivision:
                    if (kRhs == 0) return std::nullopt;
                    return ConstantValue{PinDataType::kInt, kLhs / kRhs};
                default:
                    return std::nullopt;
            }
        }
        case OperatorType::kModulo:
            if (left.type_ == PinDataType::kInt &&
                right.type_ == PinDataType::kInt) {
                const int64_t kRhs = AsInt(right);
                if (kRhs == 0) return std::nullopt;
                return ConstantValue{PinDataType::kInt, AsInt(left) % kRhs};
            }
            return std::nullopt;
        case OperatorType::kBitwiseAnd:
        case OperatorType::kBitwiseOr:
        case OperatorType::kBitwiseXor:
        case OperatorType::kLeftShift:
        case OperatorType::kRightShift:
            if (left.type_ == PinDataType::kInt &&
                right.type_ == PinDataType::kInt) {
                const int64_t kLhs = AsInt(left);
                const int64_t kRhs = AsInt(right);
                switch (type) {
                    case OperatorType::kBitwiseAnd:
                        return ConstantValue{PinDataType::kInt, kLhs & kRhs};
                    case OperatorType::kBitwiseOr:
                        return ConstantValue{PinDataType::kInt, kLhs | kRhs};
                    case OperatorType::kBitwiseXor:
                        return ConstantValue{PinDataType::kInt, kLhs ^ kRhs};
                    case OperatorType::kLeftShift:
                        return ConstantValue{PinDataType::kInt, kLhs << kRhs};
                    case OperatorType::kRightShift:
                        return ConstantValue{PinDataType::kInt, kLhs >> kRhs};
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
            if (left.type_ != right.type_ &&
                !(IsNumeric(left.type_) && IsNumeric(right.type_))) {
                return std::nullopt;
            }

            bool result = false;
            if (left.type_ == PinDataType::kString &&
                right.type_ == PinDataType::kString) {
                const auto &k_lhs = std::get<std::string>(left.value_);
                const auto &k_rhs = std::get<std::string>(right.value_);
                if (type == OperatorType::kEqual) {
                    result = k_lhs == k_rhs;
                } else if (type == OperatorType::kNotEqual) {
                    result = k_lhs != k_rhs;
                } else {
                    return std::nullopt;
                }
            } else if (left.type_ == PinDataType::kBool &&
                       right.type_ == PinDataType::kBool) {
                const bool kLhs = std::get<bool>(left.value_);
                const bool kRhs = std::get<bool>(right.value_);
                switch (type) {
                    case OperatorType::kEqual:
                        result = kLhs == kRhs;
                        break;
                    case OperatorType::kNotEqual:
                        result = kLhs != kRhs;
                        break;
                    case OperatorType::kLessThan:
                        result = kLhs < kRhs;
                        break;
                    case OperatorType::kGreaterThan:
                        result = kLhs > kRhs;
                        break;
                    case OperatorType::kLessOrEqual:
                        result = kLhs <= kRhs;
                        break;
                    case OperatorType::kGreaterOrEqual:
                        result = kLhs >= kRhs;
                        break;
                    default:
                        return std::nullopt;
                }
            } else {
                const bool kUseFloat = left.type_ == PinDataType::kFloat ||
                                       right.type_ == PinDataType::kFloat;
                if (kUseFloat) {
                    const double kLhs = AsDouble(left);
                    const double kRhs = AsDouble(right);
                    switch (type) {
                        case OperatorType::kEqual:
                            result = kLhs == kRhs;
                            break;
                        case OperatorType::kNotEqual:
                            result = kLhs != kRhs;
                            break;
                        case OperatorType::kLessThan:
                            result = kLhs < kRhs;
                            break;
                        case OperatorType::kGreaterThan:
                            result = kLhs > kRhs;
                            break;
                        case OperatorType::kLessOrEqual:
                            result = kLhs <= kRhs;
                            break;
                        case OperatorType::kGreaterOrEqual:
                            result = kLhs >= kRhs;
                            break;
                        default:
                            return std::nullopt;
                    }
                } else {
                    const int64_t kLhs = AsInt(left);
                    const int64_t kRhs = AsInt(right);
                    switch (type) {
                        case OperatorType::kEqual:
                            result = kLhs == kRhs;
                            break;
                        case OperatorType::kNotEqual:
                            result = kLhs != kRhs;
                            break;
                        case OperatorType::kLessThan:
                            result = kLhs < kRhs;
                            break;
                        case OperatorType::kGreaterThan:
                            result = kLhs > kRhs;
                            break;
                        case OperatorType::kLessOrEqual:
                            result = kLhs <= kRhs;
                            break;
                        case OperatorType::kGreaterOrEqual:
                            result = kLhs >= kRhs;
                            break;
                        default:
                            return std::nullopt;
                    }
                }
            }

            return ConstantValue{PinDataType::kBool, result};
        }
        case OperatorType::kLogicalAnd:
        case OperatorType::kLogicalOr:
            if ((left.type_ == PinDataType::kBool || IsNumeric(left.type_)) &&
                (right.type_ == PinDataType::kBool || IsNumeric(right.type_))) {
                const bool kLhs = ToBool(left);
                const bool kRhs = ToBool(right);
                if (type == OperatorType::kLogicalAnd) {
                    return ConstantValue{PinDataType::kBool, kLhs && kRhs};
                }
                return ConstantValue{PinDataType::kBool, kLhs || kRhs};
            }
            return std::nullopt;
        default:
            return std::nullopt;
    }
}

static const core::NodeBase::Connection *FindParentConnection(
    const core::NodeBase &node, uint8_t pin) {
    const auto &parents = node.GetAllParents();
    const auto kIt = std::find_if(parents.begin(), parents.end(),
                                 [pin](const core::NodeBase::Connection &conn) {
                                     return conn.in_pin == pin;
                                 });
    if (kIt == parents.end()) {
        return nullptr;
    }
    return &(*kIt);
}

}  // namespace

::code_generation::CodeGeneratorFile
editor::code_generation::CodegenContext::Generate(const core::Graph &graph) {
    ::code_generation::CodeGeneratorFile file;

    // Validate the graph
    core::GraphValidator validator;
    const auto kValidationResult = validator.Validate(graph);
    if (!kValidationResult.is_valid) {
        // Graph is invalid, return empty file with error comment
        file.Line("// ERROR: Graph validation failed");
        for (const auto &error : kValidationResult.errors) {
            file.Line("// " + error.message);
        }
        return file;
    }

    // Get topologically sorted nodes
    auto sorted_nodes =
        core::TopologicalSorter::Sort(const_cast<core::Graph &>(graph));
    std::vector<const core::NodeBase *> order;
    order.reserve(sorted_nodes.size());
    for (const auto *node : sorted_nodes) {
        order.push_back(node);
    }

    file.Line("#include <iostream>");
    file.Line("#include <string>");
    file.Line("");
    file.Line("int main() {");

    std::unordered_map<uint32_t, std::string> symbol_for_node;
    std::unordered_map<uint32_t, ConstantValue> folded_value_for_node;

    auto get_operand_expr = [&](const core::NodeBase *node,
                              uint8_t pin) -> std::string {
        const auto *connection = FindParentConnection(*node, pin);
        if (connection == nullptr || !connection->IsConnected() ||
            connection->node == nullptr) {
            return DefaultCppExprFor(node->GetInputPinType(pin));
        }

        const auto kIt = symbol_for_node.find(connection->node->id());
        if (kIt != symbol_for_node.end()) {
            return kIt->second;
        }

        return DefaultCppExprFor(node->GetInputPinType(pin));
    };

    for (const auto *node : order) {
        switch (node->kind()) {
            case core::NodeBase::NodeKind::kLiteral: {
                const auto *literal =
                    static_cast<const core::LiteralNode *>(node);
                std::ostringstream line;
                line << "const " << CppTypeFor(literal->type()) << " lit_"
                     << literal->id() << " = " << LiteralToCpp(*literal) << ";";
                file.Line("    " + line.str());

                symbol_for_node[node->id()] =
                    "lit_" + std::to_string(node->id());
                if (auto constant = GetLiteralConstant(*literal)) {
                    folded_value_for_node[node->id()] = *constant;
                }
                break;
            }
            case core::NodeBase::NodeKind::kOperator: {
                const auto *op = static_cast<const core::OperatorNode *>(node);
                const PinDataType kOutputType = op->GetOutputPinType(0);
                const std::string kSymbol = "tmp_" + std::to_string(op->id());

                std::optional<ConstantValue> folded_value;
                if (op->IsUnaryOperator()) {
                    const auto *parent = FindParentConnection(*op, 0);
                    if (parent != nullptr && parent->IsConnected() &&
                        parent->node != nullptr) {
                        const auto kIt =
                            folded_value_for_node.find(parent->node->id());
                        if (kIt != folded_value_for_node.end()) {
                            folded_value =
                                FoldUnary(op->operator_type(), kIt->second);
                        }
                    }
                } else {
                    const auto *left_parent = FindParentConnection(*op, 0);
                    const auto *right_parent = FindParentConnection(*op, 1);
                    if (left_parent != nullptr && right_parent != nullptr &&
                        left_parent->IsConnected() &&
                        right_parent->IsConnected() &&
                        left_parent->node != nullptr &&
                        right_parent->node != nullptr) {
                        const auto kLeftIt =
                            folded_value_for_node.find(left_parent->node->id());
                        const auto kRightIt = folded_value_for_node.find(
                            right_parent->node->id());
                        if (kLeftIt != folded_value_for_node.end() &&
                            kRightIt != folded_value_for_node.end()) {
                            folded_value =
                                FoldBinary(op->operator_type(), kLeftIt->second,
                                           kRightIt->second);
                        }
                    }
                }

                if (folded_value.has_value()) {
                    std::ostringstream line;
                    line << "const " << CppTypeFor(kOutputType) << " " << kSymbol
                         << " = " << ConstantToCpp(*folded_value) << ";";
                    file.Line("    " + line.str());
                    folded_value_for_node[node->id()] = *folded_value;
                    symbol_for_node[node->id()] = kSymbol;
                    break;
                }

                if (op->IsUnaryOperator()) {
                    const std::string kOperand = get_operand_expr(op, 0);
                    std::ostringstream line;
                    line << CppTypeFor(kOutputType) << " " << kSymbol << " = "
                         << OpSymbol(op->operator_type()) << kOperand << ";";
                    file.Line("    " + line.str());
                } else {
                    const std::string kLeft = get_operand_expr(op, 0);
                    const std::string kRight = get_operand_expr(op, 1);
                    std::ostringstream line;
                    line << CppTypeFor(kOutputType) << " " << kSymbol << " = "
                         << kLeft << " " << OpSymbol(op->operator_type()) << " "
                         << kRight << ";";
                    file.Line("    " + line.str());
                }

                symbol_for_node[node->id()] = kSymbol;
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

::code_generation::CodeGeneratorFile
editor::code_generation::CodegenContext::GenerateWithOutputs(
    const core::Graph &graph, bool print_all_results) {
    ::code_generation::CodeGeneratorFile file;

    // Validate the graph
    core::GraphValidator validator;
    const auto kValidationResult = validator.Validate(graph);
    if (!kValidationResult.is_valid) {
        // Graph is invalid, return empty file with error comment
        file.Line("// ERROR: Graph validation failed");
        for (const auto &error : kValidationResult.errors) {
            file.Line("// " + error.message);
        }
        return file;
    }

    // Analyze graph for dead code and type information
    CodegenOptimizer optimizer;
    auto analysis = optimizer.AnalyzeGraph(graph);

    // Get topologically sorted nodes
    auto sorted_nodes =
        core::TopologicalSorter::Sort(const_cast<core::Graph &>(graph));
    std::vector<const core::NodeBase *> order;
    order.reserve(sorted_nodes.size());
    for (const auto *node : sorted_nodes) {
        order.push_back(node);
    }

    file.Line("#include <iostream>");
    file.Line("#include <string>");
    file.Line("");
    file.Line("int main() {");

    std::unordered_map<uint32_t, std::string> symbol_for_node;
    std::unordered_map<uint32_t, ConstantValue> folded_value_for_node;
    uint32_t output_count = 0;

    auto get_operand_expr = [&](const core::NodeBase *node,
                              uint8_t pin) -> std::string {
        const auto *connection = FindParentConnection(*node, pin);
        if (connection == nullptr || !connection->IsConnected() ||
            connection->node == nullptr) {
            return DefaultCppExprFor(node->GetInputPinType(pin));
        }

        const auto kIt = symbol_for_node.find(connection->node->id());
        if (kIt != symbol_for_node.end()) {
            return kIt->second;
        }

        return DefaultCppExprFor(node->GetInputPinType(pin));
    };

    // Computation section
    file.Line("    // ===== Computation =====");
    for (const auto *node : order) {
        switch (node->kind()) {
            case core::NodeBase::NodeKind::kLiteral: {
                const auto *literal =
                    static_cast<const core::LiteralNode *>(node);
                std::ostringstream line;
                line << "const " << CppTypeFor(literal->type()) << " lit_"
                     << literal->id() << " = " << LiteralToCpp(*literal) << ";";
                file.Line("    " + line.str());

                symbol_for_node[node->id()] =
                    "lit_" + std::to_string(node->id());
                if (auto constant = GetLiteralConstant(*literal)) {
                    folded_value_for_node[node->id()] = *constant;
                }
                break;
            }
            case core::NodeBase::NodeKind::kOperator: {
                const auto *op = static_cast<const core::OperatorNode *>(node);
                const PinDataType kOutputType = op->GetOutputPinType(0);
                const std::string kSymbol =
                    "result_" + std::to_string(output_count++);

                std::optional<ConstantValue> folded_value;
                if (op->IsUnaryOperator()) {
                    const auto *parent = FindParentConnection(*op, 0);
                    if (parent != nullptr && parent->IsConnected() &&
                        parent->node != nullptr) {
                        const auto kIt =
                            folded_value_for_node.find(parent->node->id());
                        if (kIt != folded_value_for_node.end()) {
                            folded_value =
                                FoldUnary(op->operator_type(), kIt->second);
                        }
                    }
                } else {
                    const auto *left_parent = FindParentConnection(*op, 0);
                    const auto *right_parent = FindParentConnection(*op, 1);
                    if (left_parent != nullptr && right_parent != nullptr &&
                        left_parent->IsConnected() &&
                        right_parent->IsConnected() &&
                        left_parent->node != nullptr &&
                        right_parent->node != nullptr) {
                        const auto kLeftIt =
                            folded_value_for_node.find(left_parent->node->id());
                        const auto kRightIt = folded_value_for_node.find(
                            right_parent->node->id());
                        if (kLeftIt != folded_value_for_node.end() &&
                            kRightIt != folded_value_for_node.end()) {
                            folded_value =
                                FoldBinary(op->operator_type(), kLeftIt->second,
                                           kRightIt->second);
                        }
                    }
                }

                if (folded_value.has_value()) {
                    std::ostringstream line;
                    line << "const " << CppTypeFor(kOutputType) << " " << kSymbol
                         << " = " << ConstantToCpp(*folded_value) << ";";
                    file.Line("    " + line.str());
                    folded_value_for_node[node->id()] = *folded_value;
                    symbol_for_node[node->id()] = kSymbol;
                    break;
                }

                if (op->IsUnaryOperator()) {
                    const std::string kOperand = get_operand_expr(op, 0);
                    std::ostringstream line;
                    line << CppTypeFor(kOutputType) << " " << kSymbol << " = "
                         << OpSymbol(op->operator_type()) << kOperand << ";";
                    file.Line("    " + line.str());
                } else {
                    const std::string kLeft = get_operand_expr(op, 0);
                    const std::string kRight = get_operand_expr(op, 1);
                    std::ostringstream line;
                    line << CppTypeFor(kOutputType) << " " << kSymbol << " = "
                         << kLeft << " " << OpSymbol(op->operator_type()) << " "
                         << kRight << ";";
                    file.Line("    " + line.str());
                }

                symbol_for_node[node->id()] = kSymbol;
                break;
            }
            default: {
                symbol_for_node[node->id()] = "v" + std::to_string(node->id());
                break;
            }
        }
    }

    // Output section
    if (print_all_results && output_count > 0) {
        file.Line("");
        file.Line("    // ===== Results =====");
        uint32_t result_idx = 0;
        for (const auto *node : order) {
            if (node->kind() == core::NodeBase::NodeKind::kOperator) {
                const std::string kSymbol =
                    "result_" + std::to_string(result_idx++);

                std::ostringstream line;
                line << "std::cout << \"" << kSymbol << ": \" << " << kSymbol
                     << " << std::endl;";
                file.Line("    " + line.str());
            }
        }
    }

    file.Line("");
    file.Line("    return 0;");
    file.Line("}");

    return file;
}
