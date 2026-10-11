/**
 * @file c_renderer_test.cpp
 * @brief Tests of the components describing how the C nodes are edited
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#include <gtest/gtest.h>

#include <any>
#include <string>

#include "renderer/renderer.hpp"

namespace {

using c_module::renderer::ConvertLiteralValue;
using c_module::renderer::ValidateCharacter;
using c_module::renderer::ValidateIdentifier;
using c_module::renderer::ValidateString;
using c_module::renderer::node_types::kBinaryExpression;
using c_module::renderer::node_types::kConditional;
using c_module::renderer::node_types::kDeclaration;
using c_module::renderer::node_types::kFunction;
using c_module::renderer::node_types::kLiteral;
using c_module::renderer::node_types::kLoop;
using c_module::renderer::properties::kCondition;
using c_module::renderer::properties::kName;
using c_module::renderer::properties::kOperator;
using c_module::renderer::properties::kType;
using c_module::renderer::properties::kValue;
using core::Property;
using core::PropertyMap;
using core::capa::Checkbox;
using core::capa::InfoBubble;
using core::capa::NumberField;
using core::capa::RendererCapability;
using core::capa::Select;
using core::capa::TextField;

Property Text(const std::string &text) {
    return Property{.type_id_ = 1, .value_ = text};
}

PropertyMap Literal(const std::string &type, std::any value) {
    return PropertyMap{
        {kType, Text(type)},
        {kValue, Property{.type_id_ = kValue, .value_ = std::move(value)}}};
}

/// Records the last change reported by a component.
struct Recorder {
    core::NodeId node_ = 0;
    PropertyMap changes_;
    int calls_ = 0;

    c_module::renderer::PropertyUpdate Handler() {
        return [this](core::NodeId node, const PropertyMap &changes) {
            node_ = node;
            changes_ = changes;
            ++calls_;
        };
    }
};

TEST(CRendererTest, RegistersEveryEditableNodeFamily) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    EXPECT_EQ(renderer.RegisteredNodeTypeCount(), 6U);
    EXPECT_TRUE(renderer.SupportsNodeType(kDeclaration));
    EXPECT_TRUE(renderer.SupportsNodeType(kLiteral));
    EXPECT_TRUE(renderer.SupportsNodeType(kBinaryExpression));
    EXPECT_TRUE(renderer.SupportsNodeType(kConditional));
    EXPECT_TRUE(renderer.SupportsNodeType(kLoop));
    EXPECT_TRUE(renderer.SupportsNodeType(kFunction));
}

TEST(CRendererTest, DeclarationEditsItsNameAndType) {
    RendererCapability renderer;
    Recorder recorder;
    c_module::renderer::RegisterProviders(renderer, recorder.Handler());

    PropertyMap properties{{kName, Text("count")}, {kType, Text("float")}};
    auto components = renderer.GetNodeComponents(42, kDeclaration, properties);

    ASSERT_EQ(components.size(), 2U);
    auto &field = std::get<TextField>(components[0]);
    EXPECT_EQ(field.value_, "count");
    ASSERT_TRUE(field.validate_);
    EXPECT_FALSE(field.validate_("2count").empty());
    ASSERT_TRUE(field.on_change_);
    field.on_change_("total");
    EXPECT_EQ(recorder.node_, 42U);
    ASSERT_EQ(recorder.changes_.size(), 1U);
    EXPECT_EQ(std::any_cast<std::string>(recorder.changes_.at(kName).value_),
              "total");

    auto &type = std::get<Select>(components[1]);
    ASSERT_EQ(type.options_.size(), 5U);
    EXPECT_EQ(type.options_[type.selected_index_], "float");
    type.on_change_(2);  // bool
    EXPECT_EQ(std::any_cast<std::string>(recorder.changes_.at(kType).value_),
              "bool");
}

TEST(CRendererTest, MissingTypeMeansInt) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    PropertyMap properties{{kName, Text("x")}};
    auto components = renderer.GetNodeComponents(1, kDeclaration, properties);
    const auto &type = std::get<Select>(components[1]);
    EXPECT_EQ(type.options_[type.selected_index_], "int");

    properties[kType] = Text("double");  // unknown
    components = renderer.GetNodeComponents(1, kDeclaration, properties);
    EXPECT_EQ(std::get<Select>(components[1]).selected_index_, 0U);
}

TEST(CRendererTest, ReportsInvalidPropertiesAsInformation) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    auto components = renderer.GetNodeComponents(7, kLiteral, {});
    ASSERT_EQ(components.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<InfoBubble>(components[0]));
    EXPECT_TRUE(std::holds_alternative<Select>(components[1]));

    components = renderer.GetNodeComponents(7, kLiteral,
                                            Literal("int", std::string{"3"}));
    EXPECT_TRUE(std::holds_alternative<InfoBubble>(components[0]));

    components = renderer.GetNodeComponents(7, kDeclaration, {});
    EXPECT_TRUE(std::holds_alternative<InfoBubble>(components[0]));
}

TEST(CRendererTest, LiteralFieldFollowsItsType) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    auto integer = renderer.GetNodeComponents(1, kLiteral, Literal("int", 4.0));
    const auto &int_field = std::get<NumberField>(integer[0]);
    EXPECT_TRUE(int_field.is_integer_);
    EXPECT_DOUBLE_EQ(int_field.value_, 4.0);
    EXPECT_DOUBLE_EQ(int_field.min_, -2147483648.0);
    EXPECT_DOUBLE_EQ(int_field.max_, 2147483647.0);

    auto real = renderer.GetNodeComponents(1, kLiteral, Literal("float", 2.5));
    EXPECT_FALSE(std::get<NumberField>(real[0]).is_integer_);

    auto flag = renderer.GetNodeComponents(1, kLiteral, Literal("bool", true));
    EXPECT_TRUE(std::get<Checkbox>(flag[0]).value_);

    auto character = renderer.GetNodeComponents(
        1, kLiteral, Literal("char", std::string{"a"}));
    const auto &char_field = std::get<TextField>(character[0]);
    EXPECT_EQ(char_field.value_, "a");
    EXPECT_FALSE(char_field.validate_("ab").empty());

    auto text = renderer.GetNodeComponents(
        1, kLiteral, Literal("string", std::string{"Hello"}));
    EXPECT_EQ(std::get<TextField>(text[0]).value_, "Hello");
}

TEST(CRendererTest, LiteralValueChangeKeepsItsType) {
    RendererCapability renderer;
    Recorder recorder;
    c_module::renderer::RegisterProviders(renderer, recorder.Handler());

    auto components =
        renderer.GetNodeComponents(3, kLiteral, Literal("int", 4.0));
    std::get<NumberField>(components[0]).on_change_(12.0);
    ASSERT_EQ(recorder.changes_.size(), 1U);
    EXPECT_DOUBLE_EQ(std::any_cast<double>(recorder.changes_.at(kValue).value_),
                     12.0);
}

TEST(CRendererTest, LiteralTypeChangeAlsoConvertsTheValue) {
    RendererCapability renderer;
    Recorder recorder;
    c_module::renderer::RegisterProviders(renderer, recorder.Handler());

    auto components =
        renderer.GetNodeComponents(3, kLiteral, Literal("int", 4.0));
    auto &type = std::get<Select>(components[1]);

    type.on_change_(4);  // string
    ASSERT_EQ(recorder.changes_.size(), 2U);
    EXPECT_EQ(std::any_cast<std::string>(recorder.changes_.at(kType).value_),
              "string");
    EXPECT_EQ(std::any_cast<std::string>(recorder.changes_.at(kValue).value_),
              "4");

    type.on_change_(2);  // bool
    EXPECT_TRUE(std::any_cast<bool>(recorder.changes_.at(kValue).value_));

    type.on_change_(1);  // float
    EXPECT_DOUBLE_EQ(std::any_cast<double>(recorder.changes_.at(kValue).value_),
                     4.0);
}

TEST(CRendererTest, OperatorOffersTheSupportedOperators) {
    RendererCapability renderer;
    Recorder recorder;
    c_module::renderer::RegisterProviders(renderer, recorder.Handler());

    PropertyMap properties{{kOperator, Text("*")}};
    auto components =
        renderer.GetNodeComponents(5, kBinaryExpression, properties);
    ASSERT_EQ(components.size(), 1U);
    auto &select = std::get<Select>(components[0]);
    EXPECT_EQ(select.options_,
              (std::vector<std::string>{"+", "-", "*", "/", "%"}));
    EXPECT_EQ(select.selected_index_, 2U);

    select.on_change_(4);
    EXPECT_EQ(
        std::any_cast<std::string>(recorder.changes_.at(kOperator).value_),
        "%");
    select.on_change_(9);  // out of range: ignored
    EXPECT_EQ(recorder.calls_, 1);
}

TEST(CRendererTest, ConditionAndLoopSettings) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    PropertyMap condition{
        {kCondition, Property{.type_id_ = 1, .value_ = true}}};
    auto conditional = renderer.GetNodeComponents(2, kConditional, condition);
    EXPECT_TRUE(std::get<Checkbox>(conditional[0]).value_);

    EXPECT_TRUE(renderer.GetNodeComponents(2, kLoop, {}).empty());
}

TEST(CRendererTest, FunctionNameIsAnIdentifier) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    PropertyMap properties{{kName, Text("my_function")}};
    auto components = renderer.GetNodeComponents(2, kFunction, properties);
    ASSERT_EQ(components.size(), 1U);
    const auto &field = std::get<TextField>(components[0]);
    EXPECT_EQ(field.value_, "my_function");
    EXPECT_FALSE(field.validate_("my function").empty());
}

TEST(CValidationTest, IdentifiersFollowTheCRules) {
    EXPECT_TRUE(ValidateIdentifier("count").empty());
    EXPECT_TRUE(ValidateIdentifier("_total2").empty());
    EXPECT_EQ(ValidateIdentifier(""), "Le nom ne peut pas être vide.");
    EXPECT_EQ(ValidateIdentifier("2x"),
              "Le nom ne peut pas commencer par un chiffre.");
    EXPECT_EQ(ValidateIdentifier("my-var"),
              "Caractère « - » interdit : utilisez des lettres, des chiffres "
              "et _.");
    EXPECT_EQ(ValidateIdentifier("int"), "« int » est un mot réservé du C++.");
    EXPECT_EQ(ValidateIdentifier("class"),
              "« class » est un mot réservé du C++.");
    EXPECT_FALSE(ValidateIdentifier("é").empty());
}

TEST(CValidationTest, CharactersAndStrings) {
    EXPECT_TRUE(ValidateCharacter("a").empty());
    EXPECT_FALSE(ValidateCharacter("").empty());
    EXPECT_FALSE(ValidateCharacter("ab").empty());
    EXPECT_FALSE(ValidateCharacter("é").empty());
    EXPECT_TRUE(ValidateString("Hello Nebula!").empty());
    EXPECT_TRUE(ValidateString("").empty());
    EXPECT_FALSE(ValidateString("a\tb").empty());
}

TEST(CValidationTest, ConvertsLiteralValuesBetweenTypes) {
    const auto kNumber = [](double value) {
        return Property{.type_id_ = kValue, .value_ = value};
    };
    EXPECT_DOUBLE_EQ(
        std::any_cast<double>(
            ConvertLiteralValue(kNumber(4.7), "float", "int").value_),
        5.0);
    EXPECT_FALSE(std::any_cast<bool>(
        ConvertLiteralValue(kNumber(0.0), "int", "bool").value_));
    EXPECT_TRUE(std::any_cast<bool>(
        ConvertLiteralValue(Text("true"), "string", "bool").value_));
    EXPECT_EQ(std::any_cast<std::string>(
                  ConvertLiteralValue(kNumber(2.5), "float", "string").value_),
              "2.5");
    EXPECT_EQ(std::any_cast<std::string>(
                  ConvertLiteralValue(kNumber(65.0), "int", "char").value_),
              "6");
    EXPECT_EQ(std::any_cast<std::string>(
                  ConvertLiteralValue(Text(""), "string", "char").value_),
              "a");
    EXPECT_DOUBLE_EQ(std::any_cast<double>(
                         ConvertLiteralValue(Text("a"), "char", "int").value_),
                     97.0);
    EXPECT_EQ(
        std::any_cast<std::string>(
            ConvertLiteralValue(Property{.type_id_ = kValue, .value_ = true},
                                "bool", "string")
                .value_),
        "true");
}

TEST(CRendererCapabilityHandlerTest, ForwardsChangesToTheApplication) {
    RendererCapability renderer;
    core::NodeId received = 0;
    c_module::renderer::RegisterProviders(
        renderer, [&renderer](core::NodeId node, const PropertyMap &changes) {
            renderer.NotifyPropertyChange(node, changes);
        });
    renderer.SetPropertyChangeHandler(
        [&received](core::NodeId node, const PropertyMap &) {
            received = node;
        });

    auto components = renderer.GetNodeComponents(
        9, kConditional,
        {{kCondition, Property{.type_id_ = 1, .value_ = false}}});
    std::get<Checkbox>(components[0]).on_change_(true);
    EXPECT_EQ(received, 9U);
}

}  // namespace
