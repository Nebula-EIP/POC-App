/**
 * @file create_module.cpp
 * @brief Hook used to retreive the C module
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 20-09-2026
 */

#include <gtest/gtest.h>

#include <any>
#include <string>

#include "renderer/renderer.hpp"

namespace {

using c_module::renderer::node_types::kBinaryExpression;
using c_module::renderer::node_types::kConditional;
using c_module::renderer::node_types::kDeclaration;
using c_module::renderer::node_types::kLiteral;
using c_module::renderer::node_types::kLoop;
using c_module::renderer::properties::kCondition;
using c_module::renderer::properties::kName;
using c_module::renderer::properties::kOperator;
using c_module::renderer::properties::kValue;
using core::Property;
using core::PropertyMap;
using core::capa::InfoBubble;
using core::capa::Label;
using core::capa::RendererCapability;

TEST(CRendererTest, RegistersEveryNodeFamily) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    EXPECT_EQ(renderer.RegisteredNodeTypeCount(), 5U);
    EXPECT_TRUE(renderer.SupportsNodeType(kDeclaration));
    EXPECT_TRUE(renderer.SupportsNodeType(kLiteral));
    EXPECT_TRUE(renderer.SupportsNodeType(kBinaryExpression));
    EXPECT_TRUE(renderer.SupportsNodeType(kConditional));
    EXPECT_TRUE(renderer.SupportsNodeType(kLoop));
}

TEST(CRendererTest, DescribesAndUpdatesDeclaration) {
    RendererCapability renderer;
    core::NodeId updated_node = 0;
    core::PropertyId updated_property = 0;
    std::string updated_value;
    c_module::renderer::RegisterProviders(
        renderer,
        [&](core::NodeId node, core::PropertyId property, Property value) {
            updated_node = node;
            updated_property = property;
            updated_value = std::any_cast<std::string>(value.value_);
        });

    PropertyMap properties{
        {kName, Property{.type_id_ = 1, .value_ = std::string{"count"}}}};
    auto components = renderer.GetNodeComponents(42, kDeclaration, properties);

    ASSERT_EQ(components.size(), 2U);
    EXPECT_EQ(std::get<Label>(components[0]).text_, "Declaration");
    auto &field = std::get<core::capa::TextField>(components[1]);
    ASSERT_TRUE(field.on_change_);
    field.on_change_("total");
    EXPECT_EQ(updated_node, 42U);
    EXPECT_EQ(updated_property, kName);
    EXPECT_EQ(updated_value, "total");
}

TEST(CRendererTest, ReportsInvalidPropertiesAsInformation) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    auto components = renderer.GetNodeComponents(7, kLiteral, {});
    ASSERT_EQ(components.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<InfoBubble>(components[1]));

    PropertyMap wrong_type{
        {kValue, Property{.type_id_ = 1, .value_ = std::string{"3"}}}};
    components = renderer.GetNodeComponents(7, kLiteral, wrong_type);
    EXPECT_TRUE(std::holds_alternative<InfoBubble>(components[1]));
}

TEST(CRendererTest, NumericAndBooleanProvidersExposeTheirBounds) {
    RendererCapability renderer;
    c_module::renderer::RegisterProviders(renderer);

    PropertyMap number{{kValue, Property{.type_id_ = 1, .value_ = 4.0}}};
    auto literal = renderer.GetNodeComponents(1, kLiteral, number);
    const auto &field = std::get<core::capa::NumberField>(literal[1]);
    EXPECT_DOUBLE_EQ(field.min_, -1000000.0);
    EXPECT_DOUBLE_EQ(field.max_, 1000000.0);

    PropertyMap condition{
        {kCondition, Property{.type_id_ = 1, .value_ = true}}};
    auto conditional = renderer.GetNodeComponents(2, kConditional, condition);
    EXPECT_TRUE(std::get<core::capa::Checkbox>(conditional[1]).value_);
}

}  // namespace
