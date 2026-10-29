/**
 * @file renderer.cpp
 * @brief Components describing how the C nodes are edited
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 29-10-2026
 */

#include "renderer.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>

namespace {

namespace c_properties = c_module::renderer::properties;
using c_module::renderer::PropertyUpdate;
using core::Property;
using core::PropertyMap;
using core::capa::ComponentList;

// Reserved words of C and C++: the generated code is C++.
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

constexpr double kIntMin = -2147483648.0;
constexpr double kIntMax = 2147483647.0;
constexpr double kFloatLimit = 3.4e38;

template <typename T>
const T *Read(const PropertyMap &properties, core::PropertyId id) {
    const auto kProperty = properties.find(id);
    if (kProperty == properties.end()) return nullptr;
    return std::any_cast<T>(&kProperty->second.value_);
}

core::capa::InfoBubble InvalidProperty(core::PropertyId id,
                                       const char *expected) {
    core::capa::InfoBubble message;
    message.id_ = 99;
    message.text_ = "Propriété " + std::to_string(id) +
                    " absente ou invalide (" + expected + " attendu)";
    return message;
}

Property MakeProperty(core::PropertyId id, std::any value) {
    return Property{.type_id_ = id, .value_ = std::move(value)};
}

template <typename T>
void Set(core::NodeId node_id, core::PropertyId property_id, T value,
         const PropertyUpdate &update) {
    if (!update) return;
    update(node_id, PropertyMap{{property_id,
                                 MakeProperty(property_id, std::move(value))}});
}

std::size_t IndexOf(std::string_view value, const auto &options) {
    const auto kPosition = std::find(options.begin(), options.end(), value);
    return kPosition == options.end()
               ? 0U
               : static_cast<std::size_t>(kPosition - options.begin());
}

/**
 * @brief Select listing the value types; the callback receives the new type
 * name.
 *
 */
core::capa::Select TypeSelect(
    const PropertyMap &properties,
    std::function<void(const std::string &)> on_type) {
    core::capa::Select select;
    select.id_ = 10;
    select.label_ = "Type";
    for (const auto kType : c_module::renderer::kValueTypes) {
        select.options_.emplace_back(kType);
    }
    select.selected_index_ = IndexOf(c_module::renderer::ValueType(properties),
                                     c_module::renderer::kValueTypes);
    select.on_change_ = [on_type = std::move(on_type)](std::size_t index) {
        if (index < c_module::renderer::kValueTypes.size()) {
            on_type(std::string{c_module::renderer::kValueTypes[index]});
        }
    };
    return select;
}

core::capa::TextField NameField(core::NodeId node_id, const std::string &name,
                                const PropertyUpdate &update) {
    core::capa::TextField field;
    field.id_ = 2;
    field.label_ = "Nom";
    field.value_ = name;
    field.validate_ = c_module::renderer::ValidateIdentifier;
    field.on_change_ = [node_id, update](const std::string &value) {
        Set(node_id, c_properties::kName, value, update);
    };
    return field;
}

ComponentList Declaration(core::NodeId node_id, core::NodeType,
                          const PropertyMap &properties,
                          const PropertyUpdate &update) {
    ComponentList components;
    const auto kName = Read<std::string>(properties, c_properties::kName);
    if (kName == nullptr) {
        components.emplace_back(InvalidProperty(c_properties::kName, "texte"));
    } else {
        components.emplace_back(NameField(node_id, *kName, update));
    }
    components.emplace_back(
        TypeSelect(properties, [node_id, update](const std::string &type) {
            Set(node_id, c_properties::kType, type, update);
        }));
    return components;
}

ComponentList Literal(core::NodeId node_id, core::NodeType,
                      const PropertyMap &properties,
                      const PropertyUpdate &update) {
    ComponentList components;
    const std::string kType = c_module::renderer::ValueType(properties);
    const auto kValue = properties.find(c_properties::kValue);

    if (kType == "int" || kType == "float") {
        const auto kNumber = Read<double>(properties, c_properties::kValue);
        if (kNumber == nullptr) {
            components.emplace_back(
                InvalidProperty(c_properties::kValue, "nombre"));
        } else {
            core::capa::NumberField field;
            field.id_ = 2;
            field.label_ = "Valeur";
            field.value_ = *kNumber;
            field.is_integer_ = kType == "int";
            field.min_ = field.is_integer_ ? kIntMin : -kFloatLimit;
            field.max_ = field.is_integer_ ? kIntMax : kFloatLimit;
            field.step_ = 1.0;
            field.on_change_ = [node_id, update](double changed) {
                Set(node_id, c_properties::kValue, changed, update);
            };
            components.emplace_back(std::move(field));
        }
    } else if (kType == "bool") {
        const auto kFlag = Read<bool>(properties, c_properties::kValue);
        if (kFlag == nullptr) {
            components.emplace_back(
                InvalidProperty(c_properties::kValue, "booléen"));
        } else {
            core::capa::Checkbox checkbox;
            checkbox.id_ = 2;
            checkbox.label_ = "Valeur";
            checkbox.value_ = *kFlag;
            checkbox.on_change_ = [node_id, update](bool changed) {
                Set(node_id, c_properties::kValue, changed, update);
            };
            components.emplace_back(std::move(checkbox));
        }
    } else {
        const auto kText = Read<std::string>(properties, c_properties::kValue);
        if (kText == nullptr) {
            components.emplace_back(
                InvalidProperty(c_properties::kValue, "texte"));
        } else {
            core::capa::TextField field;
            field.id_ = 2;
            field.label_ = "Valeur";
            field.value_ = *kText;
            field.validate_ = kType == "char"
                                  ? c_module::renderer::ValidateCharacter
                                  : c_module::renderer::ValidateString;
            field.on_change_ = [node_id, update](const std::string &value) {
                Set(node_id, c_properties::kValue, value, update);
            };
            components.emplace_back(std::move(field));
        }
    }

    // A new type also converts the value, so both change together.
    const Property kCurrent = kValue == properties.end()
                                  ? MakeProperty(c_properties::kValue, 0.0)
                                  : kValue->second;
    components.emplace_back(TypeSelect(
        properties, [node_id, update, kType, kCurrent](const std::string &to) {
            if (!update) return;
            update(node_id, PropertyMap{
                                {c_properties::kType,
                                 MakeProperty(c_properties::kType, to)},
                                {c_properties::kValue,
                                 c_module::renderer::ConvertLiteralValue(
                                     kCurrent, kType, to)},
                            });
        }));
    return components;
}

ComponentList BinaryExpression(core::NodeId node_id, core::NodeType,
                               const PropertyMap &properties,
                               const PropertyUpdate &update) {
    ComponentList components;
    const auto kOperation =
        Read<std::string>(properties, c_properties::kOperator);
    if (kOperation == nullptr) {
        components.emplace_back(
            InvalidProperty(c_properties::kOperator, "texte"));
        return components;
    }
    core::capa::Select select;
    select.id_ = 2;
    select.label_ = "Opérateur";
    for (const auto kOperator : c_module::renderer::kOperators) {
        select.options_.emplace_back(kOperator);
    }
    select.selected_index_ =
        IndexOf(*kOperation, c_module::renderer::kOperators);
    select.on_change_ = [node_id, update](std::size_t index) {
        if (index < c_module::renderer::kOperators.size()) {
            Set(node_id, c_properties::kOperator,
                std::string{c_module::renderer::kOperators[index]}, update);
        }
    };
    components.emplace_back(std::move(select));
    components.emplace_back(
        TypeSelect(properties, [node_id, update](const std::string &type) {
            Set(node_id, c_properties::kType, type, update);
        }));
    return components;
}

ComponentList Conditional(core::NodeId node_id, core::NodeType,
                          const PropertyMap &properties,
                          const PropertyUpdate &update) {
    ComponentList components;
    const auto kCondition = Read<bool>(properties, c_properties::kCondition);
    if (kCondition == nullptr) {
        components.emplace_back(
            InvalidProperty(c_properties::kCondition, "booléen"));
        return components;
    }
    core::capa::Checkbox checkbox;
    checkbox.id_ = 2;
    checkbox.label_ = "Condition";
    checkbox.value_ = *kCondition;
    checkbox.on_change_ = [node_id, update](bool changed) {
        Set(node_id, c_properties::kCondition, changed, update);
    };
    components.emplace_back(std::move(checkbox));
    return components;
}

ComponentList Function(core::NodeId node_id, core::NodeType,
                       const PropertyMap &properties,
                       const PropertyUpdate &update) {
    ComponentList components;
    const auto kName = Read<std::string>(properties, c_properties::kName);
    if (kName == nullptr) {
        components.emplace_back(InvalidProperty(c_properties::kName, "texte"));
    } else {
        components.emplace_back(NameField(node_id, *kName, update));
    }
    return components;
}

ComponentList TypedValue(core::NodeId node_id, core::NodeType,
                         const PropertyMap &properties,
                         const PropertyUpdate &update) {
    ComponentList components;
    components.emplace_back(
        TypeSelect(properties, [node_id, update](const std::string &type) {
            Set(node_id, c_properties::kType, type, update);
        }));
    return components;
}

/**
 * @brief Nodes without settings: nothing to edit.
 *
 */
ComponentList NoSettings(core::NodeId, core::NodeType, const PropertyMap &,
                         const PropertyUpdate &) {
    return {};
}

std::string FormatNumber(double value, bool integer) {
    char text[64];
    std::snprintf(text, sizeof(text), integer ? "%.0f" : "%g", value);
    return text;
}

/**
 * @brief Text written by the user for a value, whatever its stored type.
 *
 */
std::string ValueText(const Property &value, std::string_view type) {
    if (const auto *number = std::any_cast<double>(&value.value_)) {
        return FormatNumber(*number, type == "int");
    }
    if (const auto *flag = std::any_cast<bool>(&value.value_)) {
        return *flag ? "true" : "false";
    }
    if (const auto *text_value = std::any_cast<std::string>(&value.value_)) {
        return *text_value;
    }
    return {};
}

double ValueNumber(const Property &value) {
    if (const auto *number = std::any_cast<double>(&value.value_)) {
        return *number;
    }
    if (const auto *flag = std::any_cast<bool>(&value.value_)) {
        return *flag ? 1.0 : 0.0;
    }
    if (const auto *text_value = std::any_cast<std::string>(&value.value_)) {
        if (text_value->size() == 1 &&
            !std::isdigit(static_cast<unsigned char>((*text_value)[0]))) {
            return static_cast<double>(
                static_cast<unsigned char>((*text_value)[0]));
        }
        char *end = nullptr;
        const double kParsed = std::strtod(text_value->c_str(), &end);
        if (end != text_value->c_str() && *end == '\0' &&
            std::isfinite(kParsed)) {
            return kParsed;
        }
    }
    return 0.0;
}

}  // namespace

namespace c_module::renderer {

void RegisterProviders(core::capa::RendererCapability &renderer,
                       PropertyUpdate update) {
    const auto kRegister = [&renderer, &update](core::NodeType type,
                                                auto provider) {
        renderer.RegisterNodeRenderer(
            type, [update, provider](auto id, auto node_type, const auto &p) {
                return provider(id, node_type, p, update);
            });
    };
    kRegister(node_types::kDeclaration, Declaration);
    kRegister(node_types::kLiteral, Literal);
    kRegister(node_types::kBinaryExpression, BinaryExpression);
    kRegister(node_types::kConditional, Conditional);
    kRegister(node_types::kLoop, NoSettings);
    kRegister(node_types::kFunction, Function);
    kRegister(node_types::kFunctionInput, Declaration);
    kRegister(node_types::kFunctionOutput, TypedValue);
    kRegister(node_types::kPrint, TypedValue);
    kRegister(node_types::kFor, Function);
}

std::string ValueType(const core::PropertyMap &properties) {
    const auto *type = Read<std::string>(properties, properties::kType);
    if (type == nullptr || std::find(kValueTypes.begin(), kValueTypes.end(),
                                     *type) == kValueTypes.end()) {
        return std::string{kDefaultValueType};
    }
    return *type;
}

std::string ValidateIdentifier(const std::string &name) {
    if (name.empty()) {
        return "Le nom ne peut pas être vide.";
    }
    for (const char kCharacter : name) {
        const auto kByte = static_cast<unsigned char>(kCharacter);
        if (kByte >= 0x80) {
            return "Le nom ne peut contenir que des lettres sans accent, des "
                   "chiffres et _.";
        }
        if (!std::isalnum(kByte) && kCharacter != '_') {
            return std::string{"Caractère « "} + kCharacter +
                   " » interdit : utilisez des lettres, des chiffres et _.";
        }
    }
    if (std::isdigit(static_cast<unsigned char>(name.front()))) {
        return "Le nom ne peut pas commencer par un chiffre.";
    }
    if (std::find(kReservedWords.begin(), kReservedWords.end(), name) !=
        kReservedWords.end()) {
        return "« " + name + " » est un mot réservé du C++.";
    }
    return {};
}

std::string ValidateCharacter(const std::string &text) {
    if (text.size() != 1) {
        return "Un char contient exactement un caractère.";
    }
    const auto kByte = static_cast<unsigned char>(text[0]);
    if (kByte >= 0x80 || !std::isprint(kByte)) {
        return "Un char ne peut contenir qu'un caractère ASCII imprimable.";
    }
    return {};
}

std::string ValidateString(const std::string &text) {
    for (const char kCharacter : text) {
        const auto kByte = static_cast<unsigned char>(kCharacter);
        if (kByte < 0x80 && !std::isprint(kByte)) {
            return "Le texte contient un caractère de contrôle.";
        }
    }
    return {};
}

core::Property ConvertLiteralValue(const core::Property &value,
                                   std::string_view from, std::string_view to) {
    const auto kId = properties::kValue;
    if (to == "int") {
        return MakeProperty(
            kId, std::clamp(std::round(ValueNumber(value)), kIntMin, kIntMax));
    }
    if (to == "float") {
        return MakeProperty(
            kId, std::clamp(ValueNumber(value), -kFloatLimit, kFloatLimit));
    }
    if (to == "bool") {
        if (const auto *text_value =
                std::any_cast<std::string>(&value.value_)) {
            return MakeProperty(kId,
                                *text_value == "true" || *text_value == "1");
        }
        return MakeProperty(kId, ValueNumber(value) != 0.0);
    }
    const std::string kText = ValueText(value, from);
    if (to == "char") {
        return MakeProperty(
            kId, kText.empty() || !ValidateCharacter(kText.substr(0, 1)).empty()
                     ? std::string{"a"}
                     : kText.substr(0, 1));
    }
    return MakeProperty(kId, kText);
}

}  // namespace c_module::renderer
