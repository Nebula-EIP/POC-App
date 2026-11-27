/**
 * @file c_validation_capability.cpp
 * @brief Implementation of the C module graph validation capability.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 24-11-2026
 */

#include "validation/c_validation_capability.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string_view>

#include "nodes/c_node_list_capability.hpp"

namespace c_module {

namespace {

constexpr double kIntMin = -2147483648.0;
constexpr double kIntMax = 2147483647.0;
constexpr double kFloatLimit = 3.4e38;

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

[[nodiscard]] std::optional<CNodeKind> ParseNodeKind(std::string_view name) {
    if (name == "Variable") return CNodeKind::kVariable;
    if (name == "Literal") return CNodeKind::kLiteral;
    if (name == "Operator") return CNodeKind::kOperator;
    if (name == "Condition") return CNodeKind::kCondition;
    if (name == "Loop") return CNodeKind::kLoop;
    if (name == "Function") return CNodeKind::kFunction;
    if (name == "FunctionInput") return CNodeKind::kFunctionInput;
    if (name == "FunctionOutput") return CNodeKind::kFunctionOutput;
    if (name == "Print") return CNodeKind::kPrint;
    if (name == "For") return CNodeKind::kFor;
    return std::nullopt;
}

[[nodiscard]] std::string_view NodeKindToString(CNodeKind kind) {
    switch (kind) {
        case CNodeKind::kVariable:
            return "Variable";
        case CNodeKind::kLiteral:
            return "Literal";
        case CNodeKind::kOperator:
            return "Operator";
        case CNodeKind::kCondition:
            return "Condition";
        case CNodeKind::kLoop:
            return "Loop";
        case CNodeKind::kFunction:
            return "Function";
        case CNodeKind::kFunctionInput:
            return "FunctionInput";
        case CNodeKind::kFunctionOutput:
            return "FunctionOutput";
        case CNodeKind::kPrint:
            return "Print";
        case CNodeKind::kFor:
            return "For";
    }
    return "";
}

[[nodiscard]] std::optional<CValueType> ParseValueType(std::string_view type) {
    if (type == "void") return CValueType::kVoid;
    if (type == "int") return CValueType::kInt;
    if (type == "float") return CValueType::kFloat;
    if (type == "double") return CValueType::kDouble;
    if (type == "char") return CValueType::kChar;
    if (type == "bool") return CValueType::kBool;
    if (type == "string") return CValueType::kString;
    return std::nullopt;
}

[[nodiscard]] std::optional<CValueType> ParseNonVoidValueType(
    std::string_view type) {
    const auto kParsed = ParseValueType(type);
    if (!kParsed.has_value() || *kParsed == CValueType::kVoid) {
        return std::nullopt;
    }
    return kParsed;
}

[[nodiscard]] std::string_view ValueTypeToString(CValueType type) {
    switch (type) {
        case CValueType::kVoid:
            return "void";
        case CValueType::kInt:
            return "int";
        case CValueType::kFloat:
            return "float";
        case CValueType::kDouble:
            return "double";
        case CValueType::kChar:
            return "char";
        case CValueType::kBool:
            return "bool";
        case CValueType::kString:
            return "string";
    }
    return "";
}

[[nodiscard]] std::optional<COperator> ParseOperator(std::string_view op) {
    if (op == "+") return COperator::kAdd;
    if (op == "-") return COperator::kSub;
    if (op == "*") return COperator::kMul;
    if (op == "/") return COperator::kDiv;
    if (op == "%") return COperator::kMod;
    if (op == "==") return COperator::kEq;
    if (op == "!=") return COperator::kNe;
    if (op == "<") return COperator::kLt;
    if (op == "<=") return COperator::kLe;
    if (op == ">") return COperator::kGt;
    if (op == ">=") return COperator::kGe;
    if (op == "&&") return COperator::kLogicalAnd;
    if (op == "||") return COperator::kLogicalOr;
    if (op == "&") return COperator::kBitAnd;
    if (op == "|") return COperator::kBitOr;
    if (op == "^") return COperator::kBitXor;
    if (op == "<<") return COperator::kShiftLeft;
    if (op == ">>") return COperator::kShiftRight;
    return std::nullopt;
}

[[nodiscard]] std::string_view OperatorToString(COperator op) {
    switch (op) {
        case COperator::kAdd:
            return "+";
        case COperator::kSub:
            return "-";
        case COperator::kMul:
            return "*";
        case COperator::kDiv:
            return "/";
        case COperator::kMod:
            return "%";
        case COperator::kEq:
            return "==";
        case COperator::kNe:
            return "!=";
        case COperator::kLt:
            return "<";
        case COperator::kLe:
            return "<=";
        case COperator::kGt:
            return ">";
        case COperator::kGe:
            return ">=";
        case COperator::kLogicalAnd:
            return "&&";
        case COperator::kLogicalOr:
            return "||";
        case COperator::kBitAnd:
            return "&";
        case COperator::kBitOr:
            return "|";
        case COperator::kBitXor:
            return "^";
        case COperator::kShiftLeft:
            return "<<";
        case COperator::kShiftRight:
            return ">>";
    }
    return "";
}

[[nodiscard]] bool IsLogicalOperator(COperator op) {
    return op == COperator::kLogicalAnd || op == COperator::kLogicalOr;
}

[[nodiscard]] bool IsComparisonOperator(COperator op) {
    switch (op) {
        case COperator::kEq:
        case COperator::kNe:
        case COperator::kLt:
        case COperator::kLe:
        case COperator::kGt:
        case COperator::kGe:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] bool IsBitwiseOrModuloOperator(COperator op) {
    switch (op) {
        case COperator::kMod:
        case COperator::kBitAnd:
        case COperator::kBitOr:
        case COperator::kBitXor:
        case COperator::kShiftLeft:
        case COperator::kShiftRight:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] bool IsNamedNodeKind(CNodeKind kind) {
    return kind == CNodeKind::kVariable || kind == CNodeKind::kFunction ||
           kind == CNodeKind::kFunctionInput || kind == CNodeKind::kFor;
}

[[nodiscard]] bool IsTypedNodeKind(CNodeKind kind) {
    return kind == CNodeKind::kVariable || kind == CNodeKind::kLiteral ||
           kind == CNodeKind::kOperator || kind == CNodeKind::kFunctionInput ||
           kind == CNodeKind::kFunctionOutput || kind == CNodeKind::kPrint;
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

[[nodiscard]] std::string ValidateLiteralValueForType(const std::any &val,
                                                      CValueType value_type) {
    switch (value_type) {
        case CValueType::kInt: {
            const auto kNum = ExtractNumericProperty(val);
            if (!kNum.has_value() || !std::isfinite(*kNum) || *kNum < kIntMin ||
                *kNum > kIntMax) {
                return "expected a valid 32-bit integer value";
            }
            return {};
        }
        case CValueType::kFloat:
        case CValueType::kDouble: {
            const auto kNum = ExtractNumericProperty(val);
            if (!kNum.has_value() || !std::isfinite(*kNum) ||
                *kNum < -kFloatLimit || *kNum > kFloatLimit) {
                return "expected a finite floating-point number";
            }
            return {};
        }
        case CValueType::kBool:
            if (std::any_cast<bool>(&val) == nullptr) {
                return "expected a boolean value";
            }
            return {};
        case CValueType::kChar: {
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
        case CValueType::kString:
            if (std::any_cast<std::string>(&val) == nullptr) {
                return "expected a string value";
            }
            return {};
        case CValueType::kVoid:
            return "void cannot hold a literal value";
    }
    return {};
}

[[nodiscard]] bool IsOperatorCompatibleWithType(COperator op, CValueType type) {
    if (IsLogicalOperator(op)) {
        return true;
    }
    if (type == CValueType::kString) {
        return op == COperator::kAdd || IsComparisonOperator(op);
    }
    if (type == CValueType::kFloat || type == CValueType::kDouble) {
        return !IsBitwiseOrModuloOperator(op);
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

[[nodiscard]] CValueType ExpectedNodePinValueType(const core::Node &node,
                                                  CNodeKind kind,
                                                  const core::Pin &pin,
                                                  bool is_input,
                                                  CValueType fallback_type) {
    if (IsControlFlowPinName(pin.name_) &&
        !(kind == CNodeKind::kVariable &&
          (pin.name_ == "in" || pin.name_ == "out")) &&
        !(kind == CNodeKind::kLiteral && pin.name_ == "out") &&
        !(kind == CNodeKind::kFunctionInput && pin.name_ == "out")) {
        return CValueType::kVoid;
    }

    CValueType prop_type = fallback_type;
    const auto kTypeIt = node.Properties().find(properties::kType);
    if (kTypeIt != node.Properties().end()) {
        const auto *k_type_str =
            std::any_cast<std::string>(&kTypeIt->second.value_);
        if (k_type_str != nullptr) {
            const auto kParsed = ParseNonVoidValueType(*k_type_str);
            if (kParsed.has_value()) {
                prop_type = *kParsed;
            }
        }
    }

    switch (kind) {
        case CNodeKind::kVariable:
        case CNodeKind::kLiteral:
            return prop_type;
        case CNodeKind::kOperator: {
            COperator op = COperator::kAdd;
            const auto kOpIt = node.Properties().find(properties::kOperator);
            if (kOpIt != node.Properties().end()) {
                const auto *k_op_str =
                    std::any_cast<std::string>(&kOpIt->second.value_);
                if (k_op_str != nullptr) {
                    const auto kParsedOp = ParseOperator(*k_op_str);
                    if (kParsedOp.has_value()) {
                        op = *kParsedOp;
                    }
                }
            }
            if (is_input) {
                return IsLogicalOperator(op) ? CValueType::kBool : prop_type;
            }
            return (IsLogicalOperator(op) || IsComparisonOperator(op))
                       ? CValueType::kBool
                       : prop_type;
        }
        case CNodeKind::kFunctionInput:
            return (!is_input && pin.name_ == "out") ? prop_type
                                                     : fallback_type;
        case CNodeKind::kFunctionOutput:
        case CNodeKind::kPrint:
            return (is_input && pin.name_ == "value") ? prop_type
                                                      : fallback_type;
        default:
            return fallback_type;
    }
}

[[nodiscard]] std::size_t MinRequiredInputsForKind(CNodeKind kind) {
    switch (kind) {
        case CNodeKind::kOperator:
            return 2;
        case CNodeKind::kFor:
            return 3;
        case CNodeKind::kCondition:
        case CNodeKind::kLoop:
        case CNodeKind::kPrint:
        case CNodeKind::kFunctionOutput:
            return 1;
        default:
            return 0;
    }
}

[[nodiscard]] bool IsInputPinRequired(CNodeKind kind,
                                      std::string_view pin_name) {
    switch (kind) {
        case CNodeKind::kOperator:
            return true;
        case CNodeKind::kCondition:
        case CNodeKind::kLoop:
            return pin_name == "cond" || pin_name != "in";
        case CNodeKind::kFor:
            return pin_name == "init" || pin_name == "cond" ||
                   pin_name == "step" || pin_name != "in";
        case CNodeKind::kPrint:
            return pin_name == "value" || pin_name != "control";
        case CNodeKind::kFunctionOutput:
            return pin_name == "value" || pin_name != "flow";
        case CNodeKind::kFunction:
            return pin_name != "in";
        default:
            return false;
    }
}

}  // namespace

void CValidationCapability::InitializeTypes(
    const std::unordered_map<core::DataType, std::string> &types) {
    types_ = types;
}

void CValidationCapability::InitializeNodeTypes(
    const std::unordered_map<core::NodeType, std::string> &node_types) {
    node_types_ = node_types;
}

core::capa::ValidationResult CValidationCapability::Validate(
    const core::Graph &graph) const {
    core::capa::ValidationResult result;

    std::unordered_map<core::NodeId, CNodeKind> node_kinds;
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

std::optional<CNodeKind> CValidationCapability::ResolveNodeKind(
    const core::Node &node) const {
    if (!node_types_.empty()) {
        const auto kIt = node_types_.find(node.Type());
        if (kIt == node_types_.end()) {
            return std::nullopt;
        }
        return ParseNodeKind(kIt->second);
    }

    switch (node.Type()) {
        case 1:
            return CNodeKind::kVariable;
        case 2:
            return CNodeKind::kLiteral;
        case 3:
            return CNodeKind::kOperator;
        case 4:
            return CNodeKind::kCondition;
        case 5:
            return CNodeKind::kLoop;
        case 6:
            return CNodeKind::kFunction;
        case 7:
            return CNodeKind::kFunctionInput;
        case 8:
            return CNodeKind::kFunctionOutput;
        case 9:
            return CNodeKind::kPrint;
        case 10:
            return CNodeKind::kFor;
        default:
            return std::nullopt;
    }
}

std::string CValidationCapability::FormatNodeLabel(
    const core::Node &node, std::optional<CNodeKind> kind) {
    std::ostringstream oss;
    oss << "node " << node.Id();
    if (kind.has_value()) {
        oss << " ('" << NodeKindToString(*kind) << "')";
    }
    return oss.str();
}

std::optional<CValueType> CValidationCapability::ResolveDataType(
    core::DataType type) const {
    if (!types_.empty()) {
        const auto kIt = types_.find(type);
        if (kIt != types_.end()) {
            return ParseValueType(kIt->second);
        }
        return std::nullopt;
    }

    switch (type) {
        case 1:
            return CValueType::kVoid;
        case 2:
            return CValueType::kInt;
        case 3:
            return CValueType::kFloat;
        case 4:
            return CValueType::kBool;
        case 5:
            return CValueType::kChar;
        case 6:
            return CValueType::kString;
        default:
            return std::nullopt;
    }
}

std::string CValidationCapability::ResolveTypeName(core::DataType type) const {
    const auto kValueType = ResolveDataType(type);
    if (kValueType.has_value()) {
        return std::string(ValueTypeToString(*kValueType));
    }
    const auto kIt = types_.find(type);
    if (kIt != types_.end() && !kIt->second.empty()) {
        return kIt->second;
    }
    return "type#" + std::to_string(type);
}

std::vector<core::capa::ValidationError>
CValidationCapability::CheckSupportedNodesAndValues(
    const core::Graph &graph,
    std::unordered_map<core::NodeId, CNodeKind> &node_kinds) const {
    std::vector<core::capa::ValidationError> errors;
    const auto kNodeIds = SortedNodeIds(graph);

    for (const core::NodeId kId : kNodeIds) {
        const core::Node *k_node = graph.GetNode(kId);
        if (k_node == nullptr) {
            continue;
        }

        const auto kKind = ResolveNodeKind(*k_node);
        const std::string kLabel = FormatNodeLabel(*k_node, kKind);

        if (!kKind.has_value()) {
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

        node_kinds[kId] = *kKind;
        CheckNodeProperties(graph, *k_node, *kKind, kLabel, errors);
    }

    return errors;
}

void CValidationCapability::CheckNodeProperties(
    const core::Graph &graph, const core::Node &node, CNodeKind kind,
    const std::string &label,
    std::vector<core::capa::ValidationError> &errors) const {
    CheckIdentifierProperty(node, kind, label, errors);

    const CValueType kValueType =
        ResolveAndCheckTypeProperty(node, kind, label, errors);

    if (kind == CNodeKind::kLiteral) {
        CheckLiteralProperty(node, kValueType, label, errors);
    } else if (kind == CNodeKind::kOperator) {
        CheckOperatorProperties(graph, node, kValueType, label, errors);
    } else if (kind == CNodeKind::kCondition) {
        CheckConditionProperty(node, label, errors);
    }
}

void CValidationCapability::CheckIdentifierProperty(
    const core::Node &node, CNodeKind kind, const std::string &label,
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

CValueType CValidationCapability::ResolveAndCheckTypeProperty(
    const core::Node &node, CNodeKind kind, const std::string &label,
    std::vector<core::capa::ValidationError> &errors) {
    const CValueType kDefaultType =
        (kind == CNodeKind::kPrint) ? CValueType::kString : CValueType::kInt;
    if (!IsTypedNodeKind(kind)) {
        return kDefaultType;
    }

    const auto kTypeIt = node.Properties().find(properties::kType);
    if (kTypeIt == node.Properties().end()) {
        return kDefaultType;
    }

    const auto *k_type_str =
        std::any_cast<std::string>(&kTypeIt->second.value_);
    const auto kParsedType = k_type_str != nullptr
                                 ? ParseNonVoidValueType(*k_type_str)
                                 : std::nullopt;
    if (!kParsedType.has_value()) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ =
            "Incompatible type value on " + label +
            ": unsupported C++ data type" +
            (k_type_str != nullptr ? " '" + *k_type_str + "'." : ".");
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
        return kDefaultType;
    }

    return *kParsedType;
}

void CValidationCapability::CheckLiteralProperty(
    const core::Node &node, CValueType value_type, const std::string &label,
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
        err.message_ =
            "Incompatible literal value on " + label + " for type '" +
            std::string(ValueTypeToString(value_type)) + "': " + kReason + ".";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
    }
}

void CValidationCapability::CheckOperatorProperties(
    const core::Graph &graph, const core::Node &node, CValueType value_type,
    const std::string &label,
    std::vector<core::capa::ValidationError> &errors) const {
    COperator op = COperator::kAdd;
    const auto kOpIt = node.Properties().find(properties::kOperator);
    if (kOpIt != node.Properties().end()) {
        const auto *k_op_str =
            std::any_cast<std::string>(&kOpIt->second.value_);
        const auto kParsedOp =
            k_op_str != nullptr ? ParseOperator(*k_op_str) : std::nullopt;
        if (!kParsedOp.has_value()) {
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
        op = *kParsedOp;
    }

    if (!IsOperatorCompatibleWithType(op, value_type)) {
        core::capa::ValidationError err;
        err.type_ = core::capa::ValidationError::ErrorType::kIncompatibleValue;
        err.message_ = "Incompatible operator '" +
                       std::string(OperatorToString(op)) + "' for type '" +
                       std::string(ValueTypeToString(value_type)) + "' on " +
                       label + ".";
        err.involved_node_ids_.push_back(node.Id());
        errors.push_back(std::move(err));
    }

    if (op == COperator::kDiv || op == COperator::kMod) {
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
    if (k_source == nullptr ||
        ResolveNodeKind(*k_source) != CNodeKind::kLiteral) {
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
                       FormatNodeLabel(*k_source, CNodeKind::kLiteral) + ".";
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
    const std::unordered_map<core::NodeId, CNodeKind> &node_kinds) const {
    std::vector<core::capa::ValidationError> errors;
    const auto kConnIds = SortedConnectionIds(graph);

    for (const core::ConnectionId kConnId : kConnIds) {
        CheckSingleConnection(graph, kConnId, node_kinds, errors);
    }

    return errors;
}

void CValidationCapability::CheckSingleConnection(
    const core::Graph &graph, core::ConnectionId conn_id,
    const std::unordered_map<core::NodeId, CNodeKind> &node_kinds,
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
    const std::optional<CNodeKind> kFromKind =
        kFromKindIt != node_kinds.end()
            ? std::optional<CNodeKind>(kFromKindIt->second)
            : std::nullopt;
    const auto kToKindIt = node_kinds.find(k_to_node->Id());
    const std::optional<CNodeKind> kToKind =
        kToKindIt != node_kinds.end()
            ? std::optional<CNodeKind>(kToKindIt->second)
            : std::nullopt;

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

    auto out_val_type = ResolveDataType(k_out_pin->type_);
    auto in_val_type = ResolveDataType(k_in_pin->type_);

    if (kFromKind.has_value() && out_val_type.has_value()) {
        out_val_type = ExpectedNodePinValueType(
            *k_from_node, *kFromKind, *k_out_pin, false, *out_val_type);
    }
    if (kToKind.has_value() && in_val_type.has_value()) {
        in_val_type = ExpectedNodePinValueType(*k_to_node, *kToKind, *k_in_pin,
                                               true, *in_val_type);
    }

    const std::string kOutTypeName =
        out_val_type.has_value() ? std::string(ValueTypeToString(*out_val_type))
                                 : ResolveTypeName(k_out_pin->type_);
    const std::string kInTypeName =
        in_val_type.has_value() ? std::string(ValueTypeToString(*in_val_type))
                                : ResolveTypeName(k_in_pin->type_);

    const bool kTypesMatch = (k_out_pin->type_ == k_in_pin->type_) &&
                             (out_val_type == in_val_type) &&
                             out_val_type.has_value() &&
                             (k_out_pin->type_ != 0) && (k_in_pin->type_ != 0);

    if (!kTypesMatch) {
        core::capa::ValidationError err;
        err.type_ =
            core::capa::ValidationError::ErrorType::kIncompatiblePinTypes;
        err.message_ = "Incompatible pin types: " + kFromLabel + " output '" +
                       k_out_pin->name_ + "' (" + kOutTypeName + ") -> " +
                       kToLabel + " input '" + k_in_pin->name_ + "' (" +
                       kInTypeName + ").";
        err.involved_node_ids_.push_back(k_from_node->Id());
        AddUniqueNodeId(err.involved_node_ids_, k_to_node->Id());
        err.involved_pins_.push_back(
            {k_from_node->Id(), k_out_pin->id_, false});
        err.involved_pins_.push_back({k_to_node->Id(), k_in_pin->id_, true});
        errors.push_back(std::move(err));
    }
}

std::vector<core::capa::ValidationError>
CValidationCapability::CheckPinConnectivity(
    const core::Graph &graph,
    const std::unordered_map<core::NodeId, CNodeKind> &node_kinds) const {
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
        const CNodeKind kKind = kKindIt->second;
        const std::string kLabel = FormatNodeLabel(*k_node, kKind);

        CheckNodePinConnectivity(graph, *k_node, kKind, kLabel, errors);
    }

    return errors;
}

void CValidationCapability::CheckNodePinConnectivity(
    const core::Graph &graph, const core::Node &node, CNodeKind kind,
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
