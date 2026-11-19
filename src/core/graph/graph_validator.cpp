/**
 * @file graph_validator.cpp
 * @brief Implementation of the GraphValidator class for pre-codegen checks.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 15-11-2026
 */

#include "graph/graph_validator.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <cctype>
#include <cmath>
#include <exception>
#include <optional>
#include <sstream>
#include <string_view>

namespace core {

namespace {

constexpr PropertyId kPropName = 1;
constexpr PropertyId kPropValue = 2;
constexpr PropertyId kPropOperator = 3;
constexpr PropertyId kPropCondition = 4;
constexpr PropertyId kPropType = 5;

constexpr double kIntMin = -2147483648.0;
constexpr double kIntMax = 2147483647.0;
constexpr double kFloatLimit = 3.4e38;

constexpr std::array<std::string_view, 10> kSupportedNodeNames = {
    "Variable", "Literal",       "Operator",       "Condition", "Loop",
    "Function", "FunctionInput", "FunctionOutput", "Print",     "For",
};

constexpr std::array<std::string_view, 6> kSupportedValueTypes = {
    "int", "float", "double", "char", "bool", "string",
};

constexpr std::array<std::string_view, 18> kSupportedOperators = {
    "+", "-",  "*",  "/",  "%", "==", "!=", "<",  "<=",
    ">", ">=", "&&", "||", "&", "|",  "^",  "<<", ">>",
};

constexpr std::array<std::string_view, 97> kReservedWords = {
    "alignas",
    "alignof",
    "and",
    "and_eq",
    "asm",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "char8_t",
    "char16_t",
    "char32_t",
    "class",
    "compl",
    "concept",
    "const",
    "consteval",
    "constexpr",
    "constinit",
    "const_cast",
    "continue",
    "co_await",
    "co_return",
    "co_yield",
    "decltype",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "not_eq",
    "nullptr",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "requires",
    "restrict",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "thread_local",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
    "_Bool",
    "_Complex",
    "_Imaginary",
    "main",
};

bool IsSupportedNodeName(std::string_view name) {
    return std::find(kSupportedNodeNames.begin(), kSupportedNodeNames.end(),
                     name) != kSupportedNodeNames.end();
}

bool IsSupportedValueType(std::string_view type) {
    return std::find(kSupportedValueTypes.begin(), kSupportedValueTypes.end(),
                     type) != kSupportedValueTypes.end();
}

bool IsSupportedOperator(std::string_view op) {
    return std::find(kSupportedOperators.begin(), kSupportedOperators.end(),
                     op) != kSupportedOperators.end();
}

std::string ValidateCppIdentifier(const std::string &name) {
    if (name.empty()) {
        return "identifier cannot be empty";
    }
    for (const char kCh : name) {
        const auto kByte = static_cast<unsigned char>(kCh);
        if (kByte >= 0x80 || (!std::isalnum(kByte) && kCh != '_')) {
            return "identifier '" + name + "' contains invalid characters";
        }
    }
    if (std::isdigit(static_cast<unsigned char>(name.front()))) {
        return "identifier '" + name + "' cannot start with a digit";
    }
    if (std::find(kReservedWords.begin(), kReservedWords.end(), name) !=
        kReservedWords.end()) {
        return "'" + name + "' is a C++ reserved keyword";
    }
    return {};
}

std::optional<double> ExtractNumericProperty(const std::any &val) {
    const auto *k_double = std::any_cast<double>(&val);
    if (k_double != nullptr) {
        return *k_double;
    }
    const auto *k_float = std::any_cast<float>(&val);
    if (k_float != nullptr) {
        return static_cast<double>(*k_float);
    }
    const auto *k_int = std::any_cast<int>(&val);
    if (k_int != nullptr) {
        return static_cast<double>(*k_int);
    }
    const auto *k_int64 = std::any_cast<std::int64_t>(&val);
    if (k_int64 != nullptr) {
        return static_cast<double>(*k_int64);
    }
    return std::nullopt;
}

bool IsOperatorCompatibleWithType(std::string_view op, std::string_view type) {
    if (op == "&&" || op == "||") {
        return true;
    }
    if (type == "string") {
        return op == "+" || op == "==" || op == "!=" || op == "<" ||
               op == "<=" || op == ">" || op == ">=";
    }
    if (type == "float" || type == "double") {
        if (op == "%" || op == "&" || op == "|" || op == "^" || op == "<<" ||
            op == ">>") {
            return false;
        }
        return true;
    }
    return true;
}

std::vector<NodeId> SortedNodeIds(const Graph &graph) {
    std::vector<NodeId> ids;
    ids.reserve(graph.GetAllNodes().size());
    for (const auto &[kId, kNode] : graph.GetAllNodes()) {
        (void)kNode;
        ids.push_back(kId);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::vector<ConnectionId> SortedConnectionIds(const Graph &graph) {
    std::vector<ConnectionId> ids;
    ids.reserve(graph.GetAllConnections().size());
    for (const auto &[kId, kConn] : graph.GetAllConnections()) {
        (void)kConn;
        ids.push_back(kId);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

void AddUniqueNodeId(std::vector<NodeId> &list, NodeId id) {
    if (std::find(list.begin(), list.end(), id) == list.end()) {
        list.push_back(id);
    }
}

bool IsLoopNodeKind(std::string_view kind) {
    return kind == "For" || kind == "Loop";
}

bool IsLoopLatchInputPin(std::string_view kind, std::string_view pin_name) {
    if (kind == "For") {
        return pin_name == "cond" || pin_name == "step";
    }
    if (kind == "Loop") {
        return pin_name == "cond";
    }
    return false;
}

bool IsLoopLatchOutputPin(std::string_view kind, std::string_view pin_name) {
    if (kind == "For" || kind == "Loop") {
        return pin_name == "exit";
    }
    return false;
}

bool IsControlFlowPinName(std::string_view pin_name) {
    return pin_name == "in" || pin_name == "control" || pin_name == "flow" ||
           pin_name == "out" || pin_name == "body" || pin_name == "exit" ||
           pin_name == "true" || pin_name == "false";
}

std::string ExpectedNodePinTypeName(const Node &node, std::string_view kind,
                                    const Pin &pin, bool is_input,
                                    std::string_view fallback_type_name) {
    if (IsControlFlowPinName(pin.name_) &&
        !(kind == "Variable" && (pin.name_ == "in" || pin.name_ == "out")) &&
        !(kind == "Literal" && pin.name_ == "out") &&
        !(kind == "FunctionInput" && pin.name_ == "out")) {
        return "void";
    }

    std::string prop_type = std::string(fallback_type_name);
    const auto kTypeIt = node.Properties().find(kPropType);
    if (kTypeIt != node.Properties().end()) {
        const auto *k_type_str =
            std::any_cast<std::string>(&kTypeIt->second.value_);
        if (k_type_str != nullptr && IsSupportedValueType(*k_type_str)) {
            prop_type = *k_type_str;
        }
    }

    if (kind == "Variable" || kind == "Literal") {
        return prop_type;
    }
    if (kind == "Operator") {
        std::string op = "+";
        const auto kOpIt = node.Properties().find(kPropOperator);
        if (kOpIt != node.Properties().end()) {
            const auto *k_op_str =
                std::any_cast<std::string>(&kOpIt->second.value_);
            if (k_op_str != nullptr) {
                op = *k_op_str;
            }
        }
        const bool kIsLogical = (op == "&&" || op == "||");
        const bool kIsComparison = (op == "==" || op == "!=" || op == "<" ||
                                    op == "<=" || op == ">" || op == ">=");
        if (is_input) {
            return kIsLogical ? "bool" : prop_type;
        }
        return (kIsLogical || kIsComparison) ? "bool" : prop_type;
    }
    if (kind == "FunctionInput" && !is_input && pin.name_ == "out") {
        return prop_type;
    }
    if (kind == "FunctionOutput" && is_input && pin.name_ == "value") {
        return prop_type;
    }
    if (kind == "Print" && is_input && pin.name_ == "value") {
        return prop_type;
    }
    return std::string(fallback_type_name);
}

}  // namespace

bool GraphValidator::ValidationResult::HasCycleError() const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ == ValidationError::ErrorType::kCycleDetected;
        });
}

bool GraphValidator::ValidationResult::HasTypeError() const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ ==
                   ValidationError::ErrorType::kIncompatiblePinTypes;
        });
}

bool GraphValidator::ValidationResult::HasValueError() const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ == ValidationError::ErrorType::kIncompatibleValue;
        });
}

bool GraphValidator::ValidationResult::HasUnsupportedNodeError()
    const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ == ValidationError::ErrorType::kUnsupportedNode;
        });
}

bool GraphValidator::ValidationResult::HasPinConnectivityError()
    const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ ==
                       ValidationError::ErrorType::kMissingRequiredInputPin ||
                   err.type_ ==
                       ValidationError::ErrorType::kMissingRequiredOutputPin;
        });
}

GraphValidator::ValidationResult GraphValidator::Validate(
    const Graph &graph) const {
    const ValidationContext kDefaultContext{};
    return Validate(graph, kDefaultContext);
}

GraphValidator::ValidationResult GraphValidator::Validate(
    const Graph &graph, const ValidationContext &context) const {
    ValidationResult result;

    std::unordered_map<NodeId, std::string> node_kinds;
    const auto kNodeErrors =
        CheckSupportedNodesAndValues(graph, context, node_kinds);
    result.errors_.insert(result.errors_.end(), kNodeErrors.begin(),
                          kNodeErrors.end());

    const auto kCycleErrors = DetectCycles(graph, context, node_kinds);
    result.errors_.insert(result.errors_.end(), kCycleErrors.begin(),
                          kCycleErrors.end());

    const auto kTypeErrors = CheckPinCompatibility(graph, context, node_kinds);
    result.errors_.insert(result.errors_.end(), kTypeErrors.begin(),
                          kTypeErrors.end());

    const auto kConnectivityErrors =
        CheckPinConnectivity(graph, context, node_kinds);
    result.errors_.insert(result.errors_.end(), kConnectivityErrors.begin(),
                          kConnectivityErrors.end());

    result.is_valid_ = result.errors_.empty();
    return result;
}

std::string GraphValidator::ResolveNodeKind(const Node &node,
                                            const ValidationContext &context) {
    if (context.node_type_name_) {
        return context.node_type_name_(node.Type());
    }
    if (context.catalog_ != nullptr) {
        const auto kDesc = context.catalog_->GetNode(node.Type());
        if (kDesc.has_value()) {
            return kDesc->name_;
        }
        return {};
    }
    if (context.nodes_ != nullptr) {
        const auto kAvailable = context.nodes_->GetAvailableNodes();
        for (const auto &meta : kAvailable) {
            if (meta.type_ == node.Type()) {
                return meta.name_;
            }
        }
        return {};
    }

    if (node.Type() >= 1 &&
        node.Type() <= static_cast<NodeType>(kSupportedNodeNames.size())) {
        return std::string(kSupportedNodeNames[node.Type() - 1]);
    }
    return {};
}

std::string GraphValidator::FormatNodeLabel(const Node &node,
                                            const std::string &kind_name,
                                            const ValidationContext &context) {
    std::string title;
    if (context.node_title_) {
        title = context.node_title_(node.Id());
    }
    if (title.empty() || title == "?") {
        title = kind_name;
    }
    std::ostringstream oss;
    oss << "node " << node.Id();
    if (!title.empty()) {
        oss << " ('" << title << "')";
    }
    return oss.str();
}

std::string GraphValidator::ResolveTypeName(DataType type,
                                            const ValidationContext &context) {
    if (context.type_name_) {
        const std::string kResolved = context.type_name_(type);
        if (!kResolved.empty()) {
            return kResolved;
        }
    }
    if (context.types_ != nullptr) {
        const std::string_view kView = context.types_->TypeName(type);
        if (!kView.empty()) {
            return std::string(kView);
        }
    }

    switch (type) {
        case 1:
            return "void";
        case 2:
            return "int";
        case 3:
            return "float";
        case 4:
            return "double";
        case 5:
            return "char";
        case 6:
            return "bool";
        case 7:
            return "string";
        default:
            return "type#" + std::to_string(type);
    }
}

std::vector<GraphValidator::ValidationError>
GraphValidator::CheckSupportedNodesAndValues(
    const Graph &graph, const ValidationContext &context,
    std::unordered_map<NodeId, std::string> &node_kinds) const {
    std::vector<ValidationError> errors;
    const auto kNodeIds = SortedNodeIds(graph);

    for (const NodeId kId : kNodeIds) {
        const Node *k_node = graph.GetNode(kId);
        if (k_node == nullptr) {
            continue;
        }

        const std::string kKind = ResolveNodeKind(*k_node, context);
        const std::string kLabel = FormatNodeLabel(*k_node, kKind, context);

        if (kKind.empty() || !IsSupportedNodeName(kKind)) {
            ValidationError err;
            err.type_ = ValidationError::ErrorType::kUnsupportedNode;
            err.message_ = "Unsupported node: " + kLabel + " (type " +
                           std::to_string(k_node->Type()) +
                           ") cannot be converted into C++.";
            err.involved_node_ids_.push_back(kId);
            errors.push_back(std::move(err));
            continue;
        }

        node_kinds[kId] = kKind;
        const auto &props = k_node->Properties();

        // 1. Validate identifier property (kName) on named nodes
        if (kKind == "Variable" || kKind == "Function" ||
            kKind == "FunctionInput" || kKind == "For") {
            const auto kNameIt = props.find(kPropName);
            if (kNameIt != props.end()) {
                const auto *k_name_str =
                    std::any_cast<std::string>(&kNameIt->second.value_);
                if (k_name_str == nullptr) {
                    ValidationError err;
                    err.type_ = ValidationError::ErrorType::kIncompatibleValue;
                    err.message_ = "Invalid name property on " + kLabel +
                                   ": expected a text identifier.";
                    err.involved_node_ids_.push_back(kId);
                    errors.push_back(std::move(err));
                } else {
                    const std::string kIdError =
                        ValidateCppIdentifier(*k_name_str);
                    if (!kIdError.empty()) {
                        ValidationError err;
                        err.type_ =
                            ValidationError::ErrorType::kIncompatibleValue;
                        err.message_ = "Invalid identifier on " + kLabel +
                                       ": " + kIdError + ".";
                        err.involved_node_ids_.push_back(kId);
                        errors.push_back(std::move(err));
                    }
                }
            }
        }

        // 2. Validate value type property (kType) on typed nodes
        std::string resolved_value_type = (kKind == "Print") ? "string" : "int";
        if (kKind == "Variable" || kKind == "Literal" || kKind == "Operator" ||
            kKind == "FunctionInput" || kKind == "FunctionOutput" ||
            kKind == "Print") {
            const auto kTypeIt = props.find(kPropType);
            if (kTypeIt != props.end()) {
                const auto *k_type_str =
                    std::any_cast<std::string>(&kTypeIt->second.value_);
                if (k_type_str == nullptr ||
                    !IsSupportedValueType(*k_type_str)) {
                    ValidationError err;
                    err.type_ = ValidationError::ErrorType::kIncompatibleValue;
                    err.message_ =
                        "Incompatible type value on " + kLabel +
                        ": unsupported C++ data type" +
                        (k_type_str != nullptr ? " '" + *k_type_str + "'."
                                               : ".");
                    err.involved_node_ids_.push_back(kId);
                    errors.push_back(std::move(err));
                } else {
                    resolved_value_type = *k_type_str;
                }
            }
        }

        // 3. Validate Literal value (kValue) compatibility with its type
        if (kKind == "Literal") {
            const auto kValIt = props.find(kPropValue);
            if (kValIt != props.end()) {
                bool value_ok = true;
                std::string reason;
                if (resolved_value_type == "int") {
                    const auto kNum =
                        ExtractNumericProperty(kValIt->second.value_);
                    if (!kNum.has_value() || !std::isfinite(*kNum) ||
                        *kNum < kIntMin || *kNum > kIntMax) {
                        value_ok = false;
                        reason = "expected a valid 32-bit integer value";
                    }
                } else if (resolved_value_type == "float" ||
                           resolved_value_type == "double") {
                    const auto kNum =
                        ExtractNumericProperty(kValIt->second.value_);
                    if (!kNum.has_value() || !std::isfinite(*kNum) ||
                        *kNum < -kFloatLimit || *kNum > kFloatLimit) {
                        value_ok = false;
                        reason = "expected a finite floating-point number";
                    }
                } else if (resolved_value_type == "bool") {
                    const auto *k_bool =
                        std::any_cast<bool>(&kValIt->second.value_);
                    if (k_bool == nullptr) {
                        value_ok = false;
                        reason = "expected a boolean value";
                    }
                } else if (resolved_value_type == "char") {
                    const auto *k_str =
                        std::any_cast<std::string>(&kValIt->second.value_);
                    const auto *k_char =
                        std::any_cast<char>(&kValIt->second.value_);
                    if (k_char == nullptr) {
                        if (k_str == nullptr || k_str->size() != 1 ||
                            static_cast<unsigned char>((*k_str)[0]) >= 0x80 ||
                            !std::isprint(
                                static_cast<unsigned char>((*k_str)[0]))) {
                            value_ok = false;
                            reason =
                                "expected a single printable ASCII character";
                        }
                    }
                } else if (resolved_value_type == "string") {
                    const auto *k_str =
                        std::any_cast<std::string>(&kValIt->second.value_);
                    if (k_str == nullptr) {
                        value_ok = false;
                        reason = "expected a string value";
                    }
                }

                if (!value_ok) {
                    ValidationError err;
                    err.type_ = ValidationError::ErrorType::kIncompatibleValue;
                    err.message_ = "Incompatible literal value on " + kLabel +
                                   " for type '" + resolved_value_type +
                                   "': " + reason + ".";
                    err.involved_node_ids_.push_back(kId);
                    errors.push_back(std::move(err));
                }
            }
        }

        // 4. Validate Operator compatibility with operand type & div-by-zero
        if (kKind == "Operator") {
            std::string op = "+";
            const auto kOpIt = props.find(kPropOperator);
            if (kOpIt != props.end()) {
                const auto *k_op_str =
                    std::any_cast<std::string>(&kOpIt->second.value_);
                if (k_op_str == nullptr || !IsSupportedOperator(*k_op_str)) {
                    ValidationError err;
                    err.type_ = ValidationError::ErrorType::kIncompatibleValue;
                    err.message_ =
                        "Unsupported operator on " + kLabel +
                        (k_op_str != nullptr ? ": '" + *k_op_str + "'." : ".");
                    err.involved_node_ids_.push_back(kId);
                    errors.push_back(std::move(err));
                    continue;
                }
                op = *k_op_str;
            }

            if (!IsOperatorCompatibleWithType(op, resolved_value_type)) {
                ValidationError err;
                err.type_ = ValidationError::ErrorType::kIncompatibleValue;
                err.message_ = "Incompatible operator '" + op + "' for type '" +
                               resolved_value_type + "' on " + kLabel + ".";
                err.involved_node_ids_.push_back(kId);
                errors.push_back(std::move(err));
            }

            if ((op == "/" || op == "%") && k_node->InputPins().size() >= 2) {
                const Pin &k_right_pin = k_node->InputPins()[1];
                const auto kRightConns =
                    graph.PinConnections(kId, k_right_pin.id_, true);
                if (!kRightConns.empty()) {
                    const Connection &k_conn =
                        graph.GetAllConnections().at(kRightConns.front());
                    const Node *k_source = graph.GetNode(k_conn.from_node_);
                    if (k_source != nullptr &&
                        ResolveNodeKind(*k_source, context) == "Literal") {
                        const auto kValIt =
                            k_source->Properties().find(kPropValue);
                        if (kValIt != k_source->Properties().end()) {
                            const auto kNum =
                                ExtractNumericProperty(kValIt->second.value_);
                            if (kNum.has_value() && *kNum == 0.0) {
                                ValidationError err;
                                err.type_ = ValidationError::ErrorType::
                                    kIncompatibleValue;
                                err.message_ =
                                    "Division or modulo by zero on " + kLabel +
                                    " from " +
                                    FormatNodeLabel(*k_source, "Literal",
                                                    context) +
                                    ".";
                                err.involved_node_ids_.push_back(kId);
                                AddUniqueNodeId(err.involved_node_ids_,
                                                k_source->Id());
                                err.involved_pins_.push_back(
                                    {kId, k_right_pin.id_, true});
                                errors.push_back(std::move(err));
                            }
                        }
                    }
                }
            }
        }

        // 5. Validate Condition property type if present
        if (kKind == "Condition") {
            const auto kCondIt = props.find(kPropCondition);
            if (kCondIt != props.end()) {
                const auto *k_bool =
                    std::any_cast<bool>(&kCondIt->second.value_);
                if (k_bool == nullptr) {
                    ValidationError err;
                    err.type_ = ValidationError::ErrorType::kIncompatibleValue;
                    err.message_ = "Incompatible condition property on " +
                                   kLabel + ": expected a boolean value.";
                    err.involved_node_ids_.push_back(kId);
                    errors.push_back(std::move(err));
                }
            }
        }
    }

    return errors;
}

std::vector<GraphValidator::ValidationError>
GraphValidator::CheckPinCompatibility(
    const Graph &graph, const ValidationContext &context,
    const std::unordered_map<NodeId, std::string> &node_kinds) const {
    std::vector<ValidationError> errors;
    const auto kConnIds = SortedConnectionIds(graph);

    for (const ConnectionId kConnId : kConnIds) {
        const Connection &k_conn = graph.GetAllConnections().at(kConnId);
        const Node *k_from_node = graph.GetNode(k_conn.from_node_);
        const Node *k_to_node = graph.GetNode(k_conn.to_node_);

        if (k_from_node == nullptr || k_to_node == nullptr) {
            ValidationError err;
            err.type_ = ValidationError::ErrorType::kInvalidPinIndex;
            err.message_ = "Connection " + std::to_string(kConnId) +
                           " references a missing node.";
            if (k_from_node != nullptr) {
                err.involved_node_ids_.push_back(k_from_node->Id());
            }
            if (k_to_node != nullptr) {
                err.involved_node_ids_.push_back(k_to_node->Id());
            }
            errors.push_back(std::move(err));
            continue;
        }

        const Pin *k_out_pin = k_from_node->OutputPin(k_conn.out_pin_);
        const Pin *k_in_pin = k_to_node->InputPin(k_conn.in_pin_);

        const auto kFromKindIt = node_kinds.find(k_from_node->Id());
        const std::string kFromKind =
            kFromKindIt != node_kinds.end() ? kFromKindIt->second : "";
        const auto kToKindIt = node_kinds.find(k_to_node->Id());
        const std::string kToKind =
            kToKindIt != node_kinds.end() ? kToKindIt->second : "";

        const std::string kFromLabel =
            FormatNodeLabel(*k_from_node, kFromKind, context);
        const std::string kToLabel =
            FormatNodeLabel(*k_to_node, kToKind, context);

        if (k_out_pin == nullptr || k_in_pin == nullptr) {
            ValidationError err;
            err.type_ = ValidationError::ErrorType::kInvalidPinIndex;
            err.message_ = "Connection between " + kFromLabel + " and " +
                           kToLabel + " references a missing pin.";
            err.involved_node_ids_.push_back(k_from_node->Id());
            AddUniqueNodeId(err.involved_node_ids_, k_to_node->Id());
            errors.push_back(std::move(err));
            continue;
        }

        std::string out_type_name = ResolveTypeName(k_out_pin->type_, context);
        std::string in_type_name = ResolveTypeName(k_in_pin->type_, context);

        if (!kFromKind.empty()) {
            out_type_name = ExpectedNodePinTypeName(
                *k_from_node, kFromKind, *k_out_pin, false, out_type_name);
        }
        if (!kToKind.empty()) {
            in_type_name = ExpectedNodePinTypeName(
                *k_to_node, kToKind, *k_in_pin, true, in_type_name);
        }

        bool types_match = (k_out_pin->type_ == k_in_pin->type_) &&
                           (out_type_name == in_type_name) &&
                           (k_out_pin->type_ != 0) && (k_in_pin->type_ != 0);

        if (context.nodes_ != nullptr && types_match) {
            try {
                if (!kFromKind.empty()) {
                    const auto kFromConfig =
                        context.nodes_->ResolveNodeConfiguration(
                            k_from_node->Type(), k_from_node->Properties());
                    for (const auto &cfg_pin : kFromConfig.output_pins_) {
                        if (cfg_pin.name_ == k_out_pin->name_ &&
                            cfg_pin.type_ != k_out_pin->type_) {
                            types_match = false;
                            out_type_name =
                                ResolveTypeName(cfg_pin.type_, context);
                        }
                    }
                }
                if (!kToKind.empty()) {
                    const auto kToConfig =
                        context.nodes_->ResolveNodeConfiguration(
                            k_to_node->Type(), k_to_node->Properties());
                    for (const auto &cfg_pin : kToConfig.input_pins_) {
                        if (cfg_pin.name_ == k_in_pin->name_ &&
                            cfg_pin.type_ != k_in_pin->type_) {
                            types_match = false;
                            in_type_name =
                                ResolveTypeName(cfg_pin.type_, context);
                        }
                    }
                }
            } catch (const std::exception &) {
                // Handled by unsupported node check
            }
        }

        if (!types_match) {
            ValidationError err;
            err.type_ = ValidationError::ErrorType::kIncompatiblePinTypes;
            err.message_ = "Incompatible pin types: " + kFromLabel +
                           " output '" + k_out_pin->name_ + "' (" +
                           out_type_name + ") -> " + kToLabel + " input '" +
                           k_in_pin->name_ + "' (" + in_type_name + ").";
            err.involved_node_ids_.push_back(k_from_node->Id());
            AddUniqueNodeId(err.involved_node_ids_, k_to_node->Id());
            err.involved_pins_.push_back(
                {k_from_node->Id(), k_out_pin->id_, false});
            err.involved_pins_.push_back(
                {k_to_node->Id(), k_in_pin->id_, true});
            errors.push_back(std::move(err));
        }
    }

    return errors;
}

std::vector<GraphValidator::ValidationError>
GraphValidator::CheckPinConnectivity(
    const Graph &graph, const ValidationContext &context,
    const std::unordered_map<NodeId, std::string> &node_kinds) const {
    std::vector<ValidationError> errors;
    const auto kNodeIds = SortedNodeIds(graph);

    for (const NodeId kId : kNodeIds) {
        const Node *k_node = graph.GetNode(kId);
        if (k_node == nullptr) {
            continue;
        }

        const auto kKindIt = node_kinds.find(kId);
        if (kKindIt == node_kinds.end()) {
            continue;
        }
        const std::string &k_kind = kKindIt->second;
        const std::string kLabel = FormatNodeLabel(*k_node, k_kind, context);

        // Check if node is missing expected pins altogether
        std::size_t min_required_inputs = 0;
        if (k_kind == "Operator") {
            min_required_inputs = 2;
        } else if (k_kind == "For") {
            min_required_inputs = 3;
        } else if (k_kind == "Condition" || k_kind == "Loop" ||
                   k_kind == "Print" || k_kind == "FunctionOutput") {
            min_required_inputs = 1;
        }

        if (k_node->InputPins().size() < min_required_inputs) {
            ValidationError err;
            err.type_ = ValidationError::ErrorType::kMissingRequiredInputPin;
            err.message_ = "Missing required input pin(s) on " + kLabel + ".";
            err.involved_node_ids_.push_back(kId);
            errors.push_back(std::move(err));
            continue;
        }

        for (const Pin &k_pin : k_node->InputPins()) {
            bool is_required = false;
            if (k_kind == "Operator") {
                is_required = true;
            } else if (k_kind == "Condition" || k_kind == "Loop") {
                is_required = (k_pin.name_ == "cond" || k_pin.name_ != "in");
            } else if (k_kind == "For") {
                is_required = (k_pin.name_ == "init" || k_pin.name_ == "cond" ||
                               k_pin.name_ == "step" || k_pin.name_ != "in");
            } else if (k_kind == "Print") {
                is_required =
                    (k_pin.name_ == "value" || k_pin.name_ != "control");
            } else if (k_kind == "FunctionOutput") {
                is_required = (k_pin.name_ == "value" || k_pin.name_ != "flow");
            } else if (k_kind == "Function") {
                is_required = (k_pin.name_ != "in");
            }

            if (is_required &&
                graph.PinConnections(kId, k_pin.id_, true).empty()) {
                ValidationError err;
                err.type_ =
                    ValidationError::ErrorType::kMissingRequiredInputPin;
                err.message_ = "Missing required connection on " + kLabel +
                               ": input pin '" + k_pin.name_ +
                               "' is not connected.";
                err.involved_node_ids_.push_back(kId);
                err.involved_pins_.push_back({kId, k_pin.id_, true});
                errors.push_back(std::move(err));
            }
        }
    }

    return errors;
}

std::vector<GraphValidator::ValidationError> GraphValidator::DetectCycles(
    const Graph &graph, const ValidationContext &context,
    const std::unordered_map<NodeId, std::string> &node_kinds) const {
    std::vector<ValidationError> errors;

    // 1. Check direct self-loops (from_node == to_node)
    const auto kConnIds = SortedConnectionIds(graph);
    std::vector<ConnectionId> non_self_conns;
    non_self_conns.reserve(kConnIds.size());

    for (const ConnectionId kConnId : kConnIds) {
        const Connection &k_conn = graph.GetAllConnections().at(kConnId);
        if (k_conn.from_node_ == k_conn.to_node_) {
            const Node *k_node = graph.GetNode(k_conn.from_node_);
            const auto kKindIt = node_kinds.find(k_conn.from_node_);
            const std::string kKind =
                kKindIt != node_kinds.end() ? kKindIt->second : "";
            const std::string kLabel =
                k_node != nullptr ? FormatNodeLabel(*k_node, kKind, context)
                                  : "node " + std::to_string(k_conn.from_node_);

            ValidationError err;
            err.type_ = ValidationError::ErrorType::kCycleDetected;
            err.message_ = "Circular dependency detected: " + kLabel +
                           " is connected to itself.";
            err.involved_node_ids_.push_back(k_conn.from_node_);
            err.involved_pins_.push_back(
                {k_conn.from_node_, k_conn.out_pin_, false});
            err.involved_pins_.push_back(
                {k_conn.to_node_, k_conn.in_pin_, true});
            errors.push_back(std::move(err));
        } else {
            non_self_conns.push_back(kConnId);
        }
    }

    // 2. Build 2-phase directed graph so valid loop condition/step expressions
    //    (e.g. For.index -> Operator -> For.cond) are not rejected as cycles,
    //    while invalid cycles (e.g. A -> B -> A, For.index -> Op -> For.init,
    //    For.exit -> Node -> For.in) are caught.
    struct DirectedEdge {
        std::uint64_t to_vertex_ = 0;
        ConnectionId conn_id_ = 0;
        bool is_internal_ = false;
    };

    std::unordered_map<std::uint64_t, std::vector<DirectedEdge>> adj;
    const auto kNodeIds = SortedNodeIds(graph);
    std::vector<std::uint64_t> all_vertices;
    all_vertices.reserve(kNodeIds.size() * 2);

    for (const NodeId kId : kNodeIds) {
        const auto kV0 = static_cast<std::uint64_t>(kId) * 2U;
        const auto kV1 = kV0 + 1U;
        all_vertices.push_back(kV0);

        const auto kKindIt = node_kinds.find(kId);
        const std::string kKind =
            kKindIt != node_kinds.end() ? kKindIt->second : "";
        if (IsLoopNodeKind(kKind)) {
            all_vertices.push_back(kV1);
            adj[kV0].push_back(DirectedEdge{
                .to_vertex_ = kV1, .conn_id_ = 0, .is_internal_ = true});
        }
    }

    for (const ConnectionId kConnId : non_self_conns) {
        const Connection &k_conn = graph.GetAllConnections().at(kConnId);
        const Node *k_from_node = graph.GetNode(k_conn.from_node_);
        const Node *k_to_node = graph.GetNode(k_conn.to_node_);
        if (k_from_node == nullptr || k_to_node == nullptr) {
            continue;
        }

        const Pin *k_out_pin = k_from_node->OutputPin(k_conn.out_pin_);
        const Pin *k_in_pin = k_to_node->InputPin(k_conn.in_pin_);
        if (k_out_pin == nullptr || k_in_pin == nullptr) {
            continue;
        }

        const auto kFromKindIt = node_kinds.find(k_from_node->Id());
        const std::string kFromKind =
            kFromKindIt != node_kinds.end() ? kFromKindIt->second : "";
        const auto kToKindIt = node_kinds.find(k_to_node->Id());
        const std::string kToKind =
            kToKindIt != node_kinds.end() ? kToKindIt->second : "";

        const std::uint64_t kFromPhase =
            IsLoopLatchOutputPin(kFromKind, k_out_pin->name_) ? 1U : 0U;
        const std::uint64_t kToPhase =
            IsLoopLatchInputPin(kToKind, k_in_pin->name_) ? 1U : 0U;

        const std::uint64_t kFromVertex =
            (static_cast<std::uint64_t>(k_from_node->Id()) * 2U) + kFromPhase;
        const std::uint64_t kToVertex =
            (static_cast<std::uint64_t>(k_to_node->Id()) * 2U) + kToPhase;

        adj[kFromVertex].push_back(DirectedEdge{.to_vertex_ = kToVertex,
                                                .conn_id_ = kConnId,
                                                .is_internal_ = false});
    }

    std::unordered_map<std::uint64_t, DFSColor> colors;
    for (const std::uint64_t kVertex : all_vertices) {
        colors[kVertex] = DFSColor::kWhite;
    }

    struct StackFrame {
        std::uint64_t vertex_ = 0;
        ConnectionId via_conn_ = 0;
    };
    std::vector<StackFrame> active_stack;

    std::function<void(std::uint64_t)> dfs = [&](std::uint64_t u) {
        colors[u] = DFSColor::kGray;

        const auto kEdgesIt = adj.find(u);
        if (kEdgesIt != adj.end()) {
            for (const DirectedEdge &k_edge : kEdgesIt->second) {
                const std::uint64_t kV = k_edge.to_vertex_;
                if (colors[kV] == DFSColor::kGray) {
                    ValidationError err;
                    err.type_ = ValidationError::ErrorType::kCycleDetected;

                    auto cycle_start = std::find_if(
                        active_stack.begin(), active_stack.end(),
                        [kV](const StackFrame &f) { return f.vertex_ == kV; });

                    for (auto it = cycle_start; it != active_stack.end();
                         ++it) {
                        const auto kNodeId =
                            static_cast<NodeId>(it->vertex_ / 2U);
                        AddUniqueNodeId(err.involved_node_ids_, kNodeId);
                        if (it->via_conn_ != 0) {
                            const Connection &k_c =
                                graph.GetAllConnections().at(it->via_conn_);
                            err.involved_pins_.push_back(
                                {k_c.from_node_, k_c.out_pin_, false});
                            err.involved_pins_.push_back(
                                {k_c.to_node_, k_c.in_pin_, true});
                        }
                    }

                    const auto kFromId = static_cast<NodeId>(u / 2U);
                    const auto kToId = static_cast<NodeId>(kV / 2U);
                    AddUniqueNodeId(err.involved_node_ids_, kFromId);
                    AddUniqueNodeId(err.involved_node_ids_, kToId);

                    if (!k_edge.is_internal_ && k_edge.conn_id_ != 0) {
                        const Connection &k_back_conn =
                            graph.GetAllConnections().at(k_edge.conn_id_);
                        err.involved_pins_.push_back({k_back_conn.from_node_,
                                                      k_back_conn.out_pin_,
                                                      false});
                        err.involved_pins_.push_back(
                            {k_back_conn.to_node_, k_back_conn.in_pin_, true});
                    }

                    const Node *k_from_node = graph.GetNode(kFromId);
                    const Node *k_to_node = graph.GetNode(kToId);
                    const auto kFromKindIt = node_kinds.find(kFromId);
                    const std::string kFromKind =
                        kFromKindIt != node_kinds.end() ? kFromKindIt->second
                                                        : "";
                    const auto kToKindIt = node_kinds.find(kToId);
                    const std::string kToKind =
                        kToKindIt != node_kinds.end() ? kToKindIt->second : "";

                    const std::string kFromLabel =
                        k_from_node != nullptr
                            ? FormatNodeLabel(*k_from_node, kFromKind, context)
                            : "node " + std::to_string(kFromId);
                    const std::string kToLabel =
                        k_to_node != nullptr
                            ? FormatNodeLabel(*k_to_node, kToKind, context)
                            : "node " + std::to_string(kToId);

                    err.message_ = "Circular dependency detected between " +
                                   kFromLabel + " and " + kToLabel + ".";
                    errors.push_back(std::move(err));
                } else if (colors[kV] == DFSColor::kWhite) {
                    active_stack.push_back(StackFrame{
                        .vertex_ = kV, .via_conn_ = k_edge.conn_id_});
                    dfs(kV);
                    active_stack.pop_back();
                }
            }
        }

        colors[u] = DFSColor::kBlack;
    };

    for (const std::uint64_t kVertex : all_vertices) {
        if (colors[kVertex] == DFSColor::kWhite) {
            active_stack.push_back(
                StackFrame{.vertex_ = kVertex, .via_conn_ = 0});
            dfs(kVertex);
            active_stack.pop_back();
        }
    }

    return errors;
}

}  // namespace core
