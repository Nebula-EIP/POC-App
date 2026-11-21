/**
 * @file c_validation_capability.cpp
 * @brief Implementation of the C module graph validation capability.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-11-2026
 */

#include "validation/c_validation_capability.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <exception>
#include <optional>
#include <sstream>
#include <string_view>

#include "nodes/c_node_list_capability.hpp"

namespace c_module {

namespace {

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

[[nodiscard]] bool IsSupportedNodeName(std::string_view name) {
    return std::find(kSupportedNodeNames.begin(), kSupportedNodeNames.end(),
                     name) != kSupportedNodeNames.end();
}

[[nodiscard]] bool IsSupportedValueType(std::string_view type) {
    return std::find(kSupportedValueTypes.begin(), kSupportedValueTypes.end(),
                     type) != kSupportedValueTypes.end();
}

[[nodiscard]] bool IsSupportedOperator(std::string_view op) {
    return std::find(kSupportedOperators.begin(), kSupportedOperators.end(),
                     op) != kSupportedOperators.end();
}

[[nodiscard]] bool IsNamedNodeKind(std::string_view kind) {
    return kind == "Variable" || kind == "Function" ||
           kind == "FunctionInput" || kind == "For";
}

[[nodiscard]] bool IsTypedNodeKind(std::string_view kind) {
    return kind == "Variable" || kind == "Literal" || kind == "Operator" ||
           kind == "FunctionInput" || kind == "FunctionOutput" ||
           kind == "Print";
}

[[nodiscard]] std::string ValidateCppIdentifier(const std::string &name) {
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

[[nodiscard]] std::optional<double> ExtractNumericProperty(
    const std::any &val) {
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

[[nodiscard]] std::string ValidateLiteralValueForType(
    const std::any &val, std::string_view value_type) {
    if (value_type == "int") {
        const auto kNum = ExtractNumericProperty(val);
        if (!kNum.has_value() || !std::isfinite(*kNum) || *kNum < kIntMin ||
            *kNum > kIntMax) {
            return "expected a valid 32-bit integer value";
        }
        return {};
    }
    if (value_type == "float" || value_type == "double") {
        const auto kNum = ExtractNumericProperty(val);
        if (!kNum.has_value() || !std::isfinite(*kNum) ||
            *kNum < -kFloatLimit || *kNum > kFloatLimit) {
            return "expected a finite floating-point number";
        }
        return {};
    }
    if (value_type == "bool") {
        if (std::any_cast<bool>(&val) == nullptr) {
            return "expected a boolean value";
        }
        return {};
    }
    if (value_type == "char") {
        if (std::any_cast<char>(&val) != nullptr) {
            return {};
        }
        const auto *k_str = std::any_cast<std::string>(&val);
        if (k_str == nullptr || k_str->size() != 1 ||
            static_cast<unsigned char>((*k_str)[0]) >= 0x80 ||
            !std::isprint(static_cast<unsigned char>((*k_str)[0]))) {
            return "expected a single printable ASCII character";
        }
        return {};
    }
    if (value_type == "string") {
        if (std::any_cast<std::string>(&val) == nullptr) {
            return "expected a string value";
        }
    }
    return {};
}

[[nodiscard]] bool IsOperatorCompatibleWithType(std::string_view op,
                                                std::string_view type) {
    if (op == "&&" || op == "||") {
        return true;
    }
    if (type == "string") {
        return op == "+" || op == "==" || op == "!=" || op == "<" ||
               op == "<=" || op == ">" || op == ">=";
    }
    if (type == "float" || type == "double") {
        return op != "%" && op != "&" && op != "|" && op != "^" && op != "<<" &&
               op != ">>";
    }
    return true;
}

[[nodiscard]] std::vector<core::NodeId> SortedNodeIds(
    const core::Graph &graph) {
    std::vector<core::NodeId> ids;
    ids.reserve(graph.GetAllNodes().size());
    for (const auto &[kId, kNode] : graph.GetAllNodes()) {
        (void)kNode;
        ids.push_back(kId);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

[[nodiscard]] std::vector<core::ConnectionId> SortedConnectionIds(
    const core::Graph &graph) {
    std::vector<core::ConnectionId> ids;
    ids.reserve(graph.GetAllConnections().size());
    for (const auto &[kId, kConn] : graph.GetAllConnections()) {
        (void)kConn;
        ids.push_back(kId);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

void AddUniqueNodeId(std::vector<core::NodeId> &list, core::NodeId id) {
    if (std::find(list.begin(), list.end(), id) == list.end()) {
        list.push_back(id);
    }
}

[[nodiscard]] bool IsControlFlowPinName(std::string_view pin_name) {
    return pin_name == "in" || pin_name == "control" || pin_name == "flow" ||
           pin_name == "out" || pin_name == "body" || pin_name == "exit" ||
           pin_name == "true" || pin_name == "false";
}

[[nodiscard]] std::string ExpectedNodePinTypeName(
    const core::Node &node, std::string_view kind, const core::Pin &pin,
    bool is_input, std::string_view fallback_type_name) {
    if (IsControlFlowPinName(pin.name_) &&
        !(kind == "Variable" && (pin.name_ == "in" || pin.name_ == "out")) &&
        !(kind == "Literal" && pin.name_ == "out") &&
        !(kind == "FunctionInput" && pin.name_ == "out")) {
        return "void";
    }

    std::string prop_type = std::string(fallback_type_name);
    const auto kTypeIt = node.Properties().find(properties::kType);
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
        const auto kOpIt = node.Properties().find(properties::kOperator);
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

[[nodiscard]] std::size_t MinRequiredInputsForKind(std::string_view kind) {
    if (kind == "Operator") {
        return 2;
    }
    if (kind == "For") {
        return 3;
    }
    if (kind == "Condition" || kind == "Loop" || kind == "Print" ||
        kind == "FunctionOutput") {
        return 1;
    }
    return 0;
}

[[nodiscard]] bool IsInputPinRequired(std::string_view kind,
                                      std::string_view pin_name) {
    if (kind == "Operator") {
        return true;
    }
    if (kind == "Condition" || kind == "Loop") {
        return pin_name == "cond" || pin_name != "in";
    }
    if (kind == "For") {
        return pin_name == "init" || pin_name == "cond" || pin_name == "step" ||
               pin_name != "in";
    }
    if (kind == "Print") {
        return pin_name == "value" || pin_name != "control";
    }
    if (kind == "FunctionOutput") {
        return pin_name == "value" || pin_name != "flow";
    }
    if (kind == "Function") {
        return pin_name != "in";
    }
    return false;
}

}  // namespace

CValidationCapability::CValidationCapability(
    const core::capa::ITypeListCapability *types_capa,
    const core::capa::INodeListCapability *nodes_capa)
    : types_capa_(types_capa), nodes_capa_(nodes_capa) {}

core::capa::ValidationResult CValidationCapability::Validate(
    const core::Graph &graph) const {
    core::capa::ValidationResult result;

    std::unordered_map<core::NodeId, std::string> node_kinds;
    const auto kNodeErrors = CheckSupportedNodesAndValues(graph, node_kinds);
    result.errors_.insert(result.errors_.end(), kNodeErrors.begin(),
                          kNodeErrors.end());

    const auto kTypeErrors = CheckPinCompatibility(graph, node_kinds);
    result.errors_.insert(result.errors_.end(), kTypeErrors.begin(),
                          kTypeErrors.end());

    const auto kConnectivityErrors = CheckPinConnectivity(graph, node_kinds);
    result.errors_.insert(result.errors_.end(), kConnectivityErrors.begin(),
                          kConnectivityErrors.end());

    result.is_valid_ = result.errors_.empty();
    return result;
}

std::string CValidationCapability::ResolveNodeKind(
    const core::Node &node) const {
    if (nodes_capa_ != nullptr) {
        const auto kAvailable = nodes_capa_->GetAvailableNodes();
        if (!kAvailable.empty()) {
            for (const auto &meta : kAvailable) {
                if (meta.type_ == node.Type()) {
                    return meta.name_;
                }
            }
            return {};
        }
    }

    if (node.Type() >= 1 && node.Type() <= static_cast<core::NodeType>(
                                               kSupportedNodeNames.size())) {
        return std::string(kSupportedNodeNames[node.Type() - 1]);
    }
    return {};
}

std::string CValidationCapability::FormatNodeLabel(const core::Node &node,
                                                   std::string_view kind) {
    std::ostringstream oss;
    oss << "node " << node.Id();
    if (!kind.empty()) {
        oss << " ('" << kind << "')";
    }
    return oss.str();
}

std::string CValidationCapability::ResolveTypeName(core::DataType type) const {
    if (types_capa_ != nullptr) {
        const std::string_view kView = types_capa_->TypeName(type);
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
            return "bool";
        case 5:
            return "char";
        case 6:
            return "string";
        default:
            return "type#" + std::to_string(type);
    }
}

std::vector<core::capa::ValidationError>
CValidationCapability::CheckSupportedNodesAndValues(
    const core::Graph &graph,
    std::unordered_map<core::NodeId, std::string> &node_kinds) const {
    std::vector<core::capa::ValidationError> errors;
    const auto kNodeIds = SortedNodeIds(graph);

    for (const core::NodeId kId : kNodeIds) {
        const core::Node *k_node = graph.GetNode(kId);
        if (k_node == nullptr) {
            continue;
        }

        const std::string kKind = ResolveNodeKind(*k_node);
        const std::string kLabel = FormatNodeLabel(*k_node, kKind);

        if (kKind.empty() || !IsSupportedNodeName(kKind)) {
            core::capa::ValidationError err;
            err.type_ =
                core::capa::ValidationError::ErrorType::kUnsupportedNode;
            err.message_ = "Unsupported node: " + kLabel + " (type " +
                           std::to_string(k_node->Type()) +
                           ") cannot be converted into C++.";
            err.involved_node_ids_.push_back(kId);
            errors.push_back(std::move(err));
            continue;
        }

        node_kinds[kId] = kKind;
        CheckNodeProperties(graph, *k_node, kKind, kLabel, errors);
    }

    return errors;
}

void CValidationCapability::CheckNodeProperties(
    const core::Graph &graph, const core::Node &node, std::string_view kind,
    const std::string &label,
    std::vector<core::capa::ValidationError> &errors) const {
    CheckIdentifierProperty(node, kind, label, errors);

    const std::string kValueType =
        ResolveAndCheckTypeProperty(node, kind, label, errors);

    if (kind == "Literal") {
        CheckLiteralProperty(node, kValueType, label, errors);
    } else if (kind == "Operator") {
        CheckOperatorProperties(graph, node, kValueType, label, errors);
    } else if (kind == "Condition") {
        CheckConditionProperty(node, label, errors);
    }
}

void CValidationCapability::CheckIdentifierProperty(
    const core::Node &node, std::string_view kind, const std::string &label,
    std::vector<core::capa::ValidationError> &errors) {
    if (!IsNamedNodeKind(kind)) {
        return;
    }

    const auto kNameIt = node.Properties().find(properties::kName);
    if (kNameIt == node.Properties().end()) {
        return;
    }

    const auto *k_name_str =
        std::any_cast<std::string>(&kNameIt->second.value_);
    if (k_name_str == nullptr) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ = "Invalid name property on " + label +
                       ": expected a text identifier.";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
        return;
    }

    const std::string kIdError = ValidateCppIdentifier(*k_name_str);
    if (!kIdError.empty()) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ = "Invalid identifier on " + label + ": " + kIdError + ".";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
    }
}

std::string CValidationCapability::ResolveAndCheckTypeProperty(
    const core::Node &node, std::string_view kind, const std::string &label,
    std::vector<core::capa::ValidationError> &errors) {
    std::string resolved_value_type = (kind == "Print") ? "string" : "int";
    if (!IsTypedNodeKind(kind)) {
        return resolved_value_type;
    }

    const auto kTypeIt = node.Properties().find(properties::kType);
    if (kTypeIt == node.Properties().end()) {
        return resolved_value_type;
    }

    const auto *k_type_str =
        std::any_cast<std::string>(&kTypeIt->second.value_);
    if (k_type_str == nullptr || !IsSupportedValueType(*k_type_str)) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ =
            "Incompatible type value on " + label +
            ": unsupported C++ data type" +
            (k_type_str != nullptr ? " '" + *k_type_str + "'." : ".");
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
        return resolved_value_type;
    }

    return *k_type_str;
}

void CValidationCapability::CheckLiteralProperty(
    const core::Node &node, std::string_view value_type,
    const std::string &label,
    std::vector<core::capa::ValidationError> &errors) {
    const auto kValIt = node.Properties().find(properties::kValue);
    if (kValIt == node.Properties().end()) {
        return;
    }

    const std::string kReason =
        ValidateLiteralValueForType(kValIt->second.value_, value_type);
    if (!kReason.empty()) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ = "Incompatible literal value on " + label +
                       " for type '" + std::string(value_type) +
                       "': " + kReason + ".";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
    }
}

void CValidationCapability::CheckOperatorProperties(
    const core::Graph &graph, const core::Node &node,
    std::string_view value_type, const std::string &label,
    std::vector<core::capa::ValidationError> &errors) const {
    std::string op = "+";
    const auto kOpIt = node.Properties().find(properties::kOperator);
    if (kOpIt != node.Properties().end()) {
        const auto *k_op_str =
            std::any_cast<std::string>(&kOpIt->second.value_);
        if (k_op_str == nullptr || !IsSupportedOperator(*k_op_str)) {
            core::capa::ValidationError err;
            err.type_ =
                core::capa::ValidationError::ErrorType::kIncompatibleValue;
            err.message_ =
                "Unsupported operator on " + label +
                (k_op_str != nullptr ? ": '" + *k_op_str + "'." : ".");
            err.involved_node_ids_.push_back(node.Id());
            errors.push_back(std::move(err));
            return;
        }
        op = *k_op_str;
    }

    if (!IsOperatorCompatibleWithType(op, value_type)) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ = "Incompatible operator '" + op + "' for type '" +
                       std::string(value_type) + "' on " + label + ".";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
    }

    if (op == "/" || op == "%") {
        CheckDivisionByZero(graph, node, label, errors);
    }
}

void CValidationCapability::CheckDivisionByZero(
    const core::Graph &graph, const core::Node &node, const std::string &label,
    std::vector<core::capa::ValidationError> &errors) const {
    if (node.InputPins().size() < 2) {
        return;
    }

    const core::Pin &k_right_pin = node.InputPins()[1];
    const auto kRightConns =
        graph.PinConnections(node.Id(), k_right_pin.id_, true);
    if (kRightConns.empty()) {
        return;
    }

    const core::Connection &k_conn =
        graph.GetAllConnections().at(kRightConns.front());
    const core::Node *k_source = graph.GetNode(k_conn.from_node_);
    if (k_source == nullptr || ResolveNodeKind(*k_source) != "Literal") {
        return;
    }

    const auto kValIt = k_source->Properties().find(properties::kValue);
    if (kValIt == k_source->Properties().end()) {
        return;
    }

    const auto kNum = ExtractNumericProperty(kValIt->second.value_);
    if (kNum.has_value() && *kNum == 0.0) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ = "Division or modulo by zero on " + label + " from " +
                       FormatNodeLabel(*k_source, "Literal") + ".";
        err.involved_node_ids_.push_back(node.Id());
        AddUniqueNodeId(err.involved_node_ids_, k_source->Id());
        err.involved_pins_.push_back({node.Id(), k_right_pin.id_, true});
        errors.push_back(std::move(err));
    }
}

void CValidationCapability::CheckConditionProperty(
    const core::Node &node, const std::string &label,
    std::vector<core::capa::ValidationError> &errors) {
    const auto kCondIt = node.Properties().find(properties::kCondition);
    if (kCondIt == node.Properties().end()) {
        return;
    }

    if (std::any_cast<bool>(&kCondIt->second.value_) == nullptr) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ = "Incompatible condition property on " + label +
                       ": expected a boolean value.";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
    }
}

std::vector<core::capa::ValidationError>
CValidationCapability::CheckPinCompatibility(
    const core::Graph &graph,
    const std::unordered_map<core::NodeId, std::string> &node_kinds) const {
    std::vector<core::capa::ValidationError> errors;
    const auto kConnIds = SortedConnectionIds(graph);

    for (const core::ConnectionId kConnId : kConnIds) {
        CheckSingleConnection(graph, kConnId, node_kinds, errors);
    }

    return errors;
}

void CValidationCapability::CheckSingleConnection(
    const core::Graph &graph, core::ConnectionId conn_id,
    const std::unordered_map<core::NodeId, std::string> &node_kinds,
    std::vector<core::capa::ValidationError> &errors) const {
    const core::Connection &k_conn = graph.GetAllConnections().at(conn_id);
    const core::Node *k_from_node = graph.GetNode(k_conn.from_node_);
    const core::Node *k_to_node = graph.GetNode(k_conn.to_node_);

    if (k_from_node == nullptr || k_to_node == nullptr) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kInvalidPinIndex;
        err.message_ = "Connection " + std::to_string(conn_id) +
                       " references a missing node.";
        if (k_from_node != nullptr) {
            err.involved_node_ids_.push_back(k_from_node->Id());
        }
        if (k_to_node != nullptr) {
            err.involved_node_ids_.push_back(k_to_node->Id());
        }
        errors.push_back(std::move(err));
        return;
    }

    const core::Pin *k_out_pin = k_from_node->OutputPin(k_conn.out_pin_);
    const core::Pin *k_in_pin = k_to_node->InputPin(k_conn.in_pin_);

    const auto kFromKindIt = node_kinds.find(k_from_node->Id());
    const std::string kFromKind =
        kFromKindIt != node_kinds.end() ? kFromKindIt->second : "";
    const auto kToKindIt = node_kinds.find(k_to_node->Id());
    const std::string kToKind =
        kToKindIt != node_kinds.end() ? kToKindIt->second : "";

    const std::string kFromLabel = FormatNodeLabel(*k_from_node, kFromKind);
    const std::string kToLabel = FormatNodeLabel(*k_to_node, kToKind);

    if (k_out_pin == nullptr || k_in_pin == nullptr) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kInvalidPinIndex;
        err.message_ = "Connection between " + kFromLabel + " and " + kToLabel +
                       " references a missing pin.";
        err.involved_node_ids_.push_back(k_from_node->Id());
        AddUniqueNodeId(err.involved_node_ids_, k_to_node->Id());
        errors.push_back(std::move(err));
        return;
    }

    std::string out_type_name = ResolveTypeName(k_out_pin->type_);
    std::string in_type_name = ResolveTypeName(k_in_pin->type_);

    if (!kFromKind.empty()) {
        out_type_name = ExpectedNodePinTypeName(
            *k_from_node, kFromKind, *k_out_pin, false, out_type_name);
    }
    if (!kToKind.empty()) {
        in_type_name = ExpectedNodePinTypeName(*k_to_node, kToKind, *k_in_pin,
                                               true, in_type_name);
    }

    bool types_match = (k_out_pin->type_ == k_in_pin->type_) &&
                       (out_type_name == in_type_name) &&
                       (k_out_pin->type_ != 0) && (k_in_pin->type_ != 0);

    if (nodes_capa_ != nullptr && types_match) {
        types_match = VerifyPinTypesAgainstModuleConfig(
            *k_from_node, kFromKind, *k_out_pin, *k_to_node, kToKind, *k_in_pin,
            out_type_name, in_type_name);
    }

    if (!types_match) {
        core::capa::ValidationError err;
        err.type_ =
            core::capa::ValidationError::ErrorType::kIncompatiblePinTypes;
        err.message_ = "Incompatible pin types: " + kFromLabel + " output '" +
                       k_out_pin->name_ + "' (" + out_type_name + ") -> " +
                       kToLabel + " input '" + k_in_pin->name_ + "' (" +
                       in_type_name + ").";
        err.involved_node_ids_.push_back(k_from_node->Id());
        AddUniqueNodeId(err.involved_node_ids_, k_to_node->Id());
        err.involved_pins_.push_back(
            {k_from_node->Id(), k_out_pin->id_, false});
        err.involved_pins_.push_back({k_to_node->Id(), k_in_pin->id_, true});
        errors.push_back(std::move(err));
    }
}

bool CValidationCapability::VerifyPinTypesAgainstModuleConfig(
    const core::Node &from_node, std::string_view from_kind,
    const core::Pin &out_pin, const core::Node &to_node,
    std::string_view to_kind, const core::Pin &in_pin,
    std::string &out_type_name, std::string &in_type_name) const {
    bool types_match = true;
    try {
        if (!from_kind.empty()) {
            const auto kFromConfig = nodes_capa_->ResolveNodeConfiguration(
                from_node.Type(), from_node.Properties());
            for (const auto &cfg_pin : kFromConfig.output_pins_) {
                if (cfg_pin.name_ == out_pin.name_ &&
                    cfg_pin.type_ != out_pin.type_) {
                    types_match = false;
                    out_type_name = ResolveTypeName(cfg_pin.type_);
                }
            }
        }
        if (!to_kind.empty()) {
            const auto kToConfig = nodes_capa_->ResolveNodeConfiguration(
                to_node.Type(), to_node.Properties());
            for (const auto &cfg_pin : kToConfig.input_pins_) {
                if (cfg_pin.name_ == in_pin.name_ &&
                    cfg_pin.type_ != in_pin.type_) {
                    types_match = false;
                    in_type_name = ResolveTypeName(cfg_pin.type_);
                }
            }
        }
    } catch (const std::exception &) {
        // Handled by unsupported node check
    }
    return types_match;
}

std::vector<core::capa::ValidationError>
CValidationCapability::CheckPinConnectivity(
    const core::Graph &graph,
    const std::unordered_map<core::NodeId, std::string> &node_kinds) const {
    std::vector<core::capa::ValidationError> errors;
    const auto kNodeIds = SortedNodeIds(graph);

    for (const core::NodeId kId : kNodeIds) {
        const core::Node *k_node = graph.GetNode(kId);
        if (k_node == nullptr) {
            continue;
        }

        const auto kKindIt = node_kinds.find(kId);
        if (kKindIt == node_kinds.end()) {
            continue;
        }
        const std::string &k_kind = kKindIt->second;
        const std::string kLabel = FormatNodeLabel(*k_node, k_kind);

        CheckNodePinConnectivity(graph, *k_node, k_kind, kLabel, errors);
    }

    return errors;
}

void CValidationCapability::CheckNodePinConnectivity(
    const core::Graph &graph, const core::Node &node, std::string_view kind,
    const std::string &label,
    std::vector<core::capa::ValidationError> &errors) const {
    const std::size_t kMinRequiredInputs = MinRequiredInputsForKind(kind);

    if (node.InputPins().size() < kMinRequiredInputs) {
        core::capa::ValidationError err;
        err.type_ =
            core::capa::ValidationError::ErrorType::kMissingRequiredInputPin;
        err.message_ = "Missing required input pin(s) on " + label + ".";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
        return;
    }

    for (const core::Pin &k_pin : node.InputPins()) {
        if (IsInputPinRequired(kind, k_pin.name_) &&
            graph.PinConnections(node.Id(), k_pin.id_, true).empty()) {
            core::capa::ValidationError err;
            err.type_ = core::capa::ValidationError::ErrorType::
                kMissingRequiredInputPin;
            err.message_ = "Missing required connection on " + label +
                           ": input pin '" + k_pin.name_ +
                           "' is not connected.";
            err.involved_node_ids_.push_back(node.Id());
            err.involved_pins_.push_back({node.Id(), k_pin.id_, true});
            errors.push_back(std::move(err));
        }
    }
}

}  // namespace c_module
