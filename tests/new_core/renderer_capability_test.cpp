#include "modules/capabilities/renderer_capability.hpp"

#include <gtest/gtest.h>

#include <any>
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "exception/graph_exception/node_exception.hpp"

namespace {

using core::NodeId;
using core::NodeType;
using core::Property;
using core::PropertyId;
using core::PropertyMap;
using core::capa::Button;
using core::capa::Checkbox;
using core::capa::Component;
using core::capa::ComponentBase;
using core::capa::ComponentList;
using core::capa::InfoBubble;
using core::capa::IRendererCapability;
using core::capa::Label;
using core::capa::NumberField;
using core::capa::Select;
using core::capa::Separator;
using core::capa::Slider;
using core::capa::TextField;

constexpr NodeType kSupportedNodeType = 7;
constexpr NodeType kUnsupportedNodeType = 8;
constexpr PropertyId kNameProperty = 12;

void ExpectDefaultBaseMetadata(const ComponentBase &component) {
    EXPECT_EQ(component.id, 0);
    EXPECT_TRUE(component.tooltip.empty());
}

class TestRendererCapability final : public IRendererCapability {
   public:
    bool SupportsNodeType(NodeType node_type) const noexcept override {
        return node_type == kSupportedNodeType;
    }

    ComponentList GetNodeComponents(NodeId node_id, NodeType node_type,
                                    const PropertyMap &properties) override {
        if (!SupportsNodeType(node_type)) {
            throw core::NodeTypeException("Unsupported renderer node type");
        }

        const auto property = properties.find(kNameProperty);
        if (property == properties.end()) {
            throw core::InvalidNodeException("Missing node name property");
        }

        const auto *name = std::any_cast<std::string>(&property->second.value);
        if (name == nullptr) {
            throw core::InvalidNodeException("Invalid node name property");
        }

        Label title;
        title.id = 1;
        title.tooltip = "Node identifier";
        title.text = "Node " + std::to_string(node_id);

        TextField name_field;
        name_field.id = 2;
        name_field.label = "Name";
        name_field.value = *name;
        name_field.on_change = [this](const std::string &new_name) {
            last_name_ = new_name;
        };

        Button reset_button;
        reset_button.id = 3;
        reset_button.label = "Reset";
        reset_button.on_click = [this] { ++reset_count_; };

        return {std::move(title), std::move(name_field),
                std::move(reset_button)};
    }

    const std::string &last_name() const noexcept { return last_name_; }
    std::size_t reset_count() const noexcept { return reset_count_; }

   private:
    std::string last_name_;
    std::size_t reset_count_ = 0;
};

static_assert(std::is_base_of_v<core::ICapability, IRendererCapability>);
static_assert(std::is_abstract_v<IRendererCapability>);
static_assert(std::has_virtual_destructor_v<IRendererCapability>);
static_assert(std::variant_size_v<Component> == 9);

TEST(RendererComponentTest, StaticComponentsHaveEmptyDefaults) {
    const Label label;
    const InfoBubble info_bubble;
    const Separator separator;

    ExpectDefaultBaseMetadata(label);
    ExpectDefaultBaseMetadata(info_bubble);
    ExpectDefaultBaseMetadata(separator);
    EXPECT_TRUE(label.text.empty());
    EXPECT_TRUE(info_bubble.text.empty());
}

TEST(RendererComponentTest, ButtonHasSafeDefaults) {
    const Button button;

    ExpectDefaultBaseMetadata(button);
    EXPECT_TRUE(button.label.empty());
    EXPECT_TRUE(button.enabled);
    EXPECT_FALSE(static_cast<bool>(button.on_click));
}

TEST(RendererComponentTest, TextFieldHasSafeDefaults) {
    const TextField field;

    ExpectDefaultBaseMetadata(field);
    EXPECT_TRUE(field.label.empty());
    EXPECT_TRUE(field.value.empty());
    EXPECT_FALSE(field.read_only);
    EXPECT_TRUE(field.enabled);
    EXPECT_FALSE(static_cast<bool>(field.on_change));
}

TEST(RendererComponentTest, NumericFieldsHaveSafeDefaults) {
    const NumberField number;
    const Slider slider;

    ExpectDefaultBaseMetadata(number);
    EXPECT_DOUBLE_EQ(number.value, 0.0);
    EXPECT_DOUBLE_EQ(number.min, 0.0);
    EXPECT_DOUBLE_EQ(number.max, 1.0);
    EXPECT_DOUBLE_EQ(number.step, 0.0);
    EXPECT_FALSE(number.is_integer);
    EXPECT_TRUE(number.enabled);
    EXPECT_FALSE(static_cast<bool>(number.on_change));

    ExpectDefaultBaseMetadata(slider);
    EXPECT_DOUBLE_EQ(slider.value, 0.0);
    EXPECT_DOUBLE_EQ(slider.min, 0.0);
    EXPECT_DOUBLE_EQ(slider.max, 1.0);
    EXPECT_DOUBLE_EQ(slider.step, 0.0);
    EXPECT_FALSE(slider.is_integer);
    EXPECT_TRUE(slider.enabled);
    EXPECT_FALSE(static_cast<bool>(slider.on_change));
}

TEST(RendererComponentTest, ChoiceFieldsHaveSafeDefaults) {
    const Checkbox checkbox;
    const Select select;

    ExpectDefaultBaseMetadata(checkbox);
    EXPECT_TRUE(checkbox.label.empty());
    EXPECT_FALSE(checkbox.value);
    EXPECT_TRUE(checkbox.enabled);
    EXPECT_FALSE(static_cast<bool>(checkbox.on_change));

    ExpectDefaultBaseMetadata(select);
    EXPECT_TRUE(select.label.empty());
    EXPECT_TRUE(select.options.empty());
    EXPECT_EQ(select.selected_index, 0U);
    EXPECT_TRUE(select.enabled);
    EXPECT_FALSE(static_cast<bool>(select.on_change));
}

TEST(RendererComponentTest, SharedMetadataAndComponentValuesAreRetained) {
    Select select;
    select.id = 42;
    select.tooltip = "Select a mode";
    select.label = "Mode";
    select.options = {"Fast", "Safe"};
    select.selected_index = 1;
    select.enabled = false;

    EXPECT_EQ(select.id, 42);
    EXPECT_EQ(select.tooltip, "Select a mode");
    EXPECT_EQ(select.label, "Mode");
    EXPECT_EQ(select.options, (std::vector<std::string>{"Fast", "Safe"}));
    EXPECT_EQ(select.selected_index, 1U);
    EXPECT_FALSE(select.enabled);
}

TEST(RendererComponentTest, InteractionCallbacksReceiveTheirValues) {
    bool clicked = false;
    std::string text_value;
    double number_value = 0.0;
    double slider_value = 0.0;
    bool checkbox_value = false;
    std::size_t selected_index = 0;

    Button button;
    button.on_click = [&clicked] { clicked = true; };
    TextField text;
    text.on_change = [&text_value](const std::string &value) {
        text_value = value;
    };
    NumberField number;
    number.on_change = [&number_value](double value) { number_value = value; };
    Slider slider;
    slider.on_change = [&slider_value](double value) { slider_value = value; };
    Checkbox checkbox;
    checkbox.on_change = [&checkbox_value](bool value) {
        checkbox_value = value;
    };
    Select select;
    select.on_change = [&selected_index](std::size_t index) {
        selected_index = index;
    };

    button.on_click();
    text.on_change("updated");
    number.on_change(2.5);
    slider.on_change(0.75);
    checkbox.on_change(true);
    select.on_change(3);

    EXPECT_TRUE(clicked);
    EXPECT_EQ(text_value, "updated");
    EXPECT_DOUBLE_EQ(number_value, 2.5);
    EXPECT_DOUBLE_EQ(slider_value, 0.75);
    EXPECT_TRUE(checkbox_value);
    EXPECT_EQ(selected_index, 3U);
}

TEST(RendererComponentTest, ComponentVariantSupportsEveryDeclaredComponent) {
    ComponentList components;
    components.emplace_back(Label{});
    components.emplace_back(Button{});
    components.emplace_back(TextField{});
    components.emplace_back(NumberField{});
    components.emplace_back(Slider{});
    components.emplace_back(Checkbox{});
    components.emplace_back(Select{});
    components.emplace_back(InfoBubble{});
    components.emplace_back(Separator{});

    ASSERT_EQ(components.size(), 9U);
    EXPECT_TRUE(std::holds_alternative<Label>(components[0]));
    EXPECT_TRUE(std::holds_alternative<Button>(components[1]));
    EXPECT_TRUE(std::holds_alternative<TextField>(components[2]));
    EXPECT_TRUE(std::holds_alternative<NumberField>(components[3]));
    EXPECT_TRUE(std::holds_alternative<Slider>(components[4]));
    EXPECT_TRUE(std::holds_alternative<Checkbox>(components[5]));
    EXPECT_TRUE(std::holds_alternative<Select>(components[6]));
    EXPECT_TRUE(std::holds_alternative<InfoBubble>(components[7]));
    EXPECT_TRUE(std::holds_alternative<Separator>(components[8]));
}

TEST(RendererCapabilityTest, ReportsSupportedNodeTypesThroughTheInterface) {
    const TestRendererCapability renderer;
    const IRendererCapability &capability = renderer;

    EXPECT_TRUE(capability.SupportsNodeType(kSupportedNodeType));
    EXPECT_FALSE(capability.SupportsNodeType(kUnsupportedNodeType));
}

TEST(RendererCapabilityTest, BuildsNodeMetadataFromProperties) {
    TestRendererCapability renderer;
    IRendererCapability &capability = renderer;
    PropertyMap properties = {
        {kNameProperty, Property{.type_id = 3, .value = std::string("Adder")}},
    };

    const ComponentList components =
        capability.GetNodeComponents(123, kSupportedNodeType, properties);

    ASSERT_EQ(components.size(), 3U);
    const auto &title = std::get<Label>(components[0]);
    const auto &name = std::get<TextField>(components[1]);
    const auto &reset = std::get<Button>(components[2]);
    EXPECT_EQ(title.id, 1);
    EXPECT_EQ(title.tooltip, "Node identifier");
    EXPECT_EQ(title.text, "Node 123");
    EXPECT_EQ(name.id, 2);
    EXPECT_EQ(name.label, "Name");
    EXPECT_EQ(name.value, "Adder");
    EXPECT_EQ(reset.id, 3);
    EXPECT_EQ(reset.label, "Reset");
}

TEST(RendererCapabilityTest, ReturnedCallbacksRemainUsable) {
    TestRendererCapability renderer;
    PropertyMap properties = {
        {kNameProperty, Property{.type_id = 3, .value = std::string("Adder")}},
    };
    ComponentList components =
        renderer.GetNodeComponents(123, kSupportedNodeType, properties);

    std::get<TextField>(components[1]).on_change("Multiplier");
    std::get<Button>(components[2]).on_click();
    std::get<Button>(components[2]).on_click();

    EXPECT_EQ(renderer.last_name(), "Multiplier");
    EXPECT_EQ(renderer.reset_count(), 2U);
}

TEST(RendererCapabilityTest, UnsupportedNodeUsesProvidedNodeTypeException) {
    TestRendererCapability renderer;
    const PropertyMap properties;

    EXPECT_THROW(
        renderer.GetNodeComponents(1, kUnsupportedNodeType, properties),
        core::NodeTypeException);
    EXPECT_THROW(
        renderer.GetNodeComponents(1, kUnsupportedNodeType, properties),
        core::Exception);
}

TEST(RendererCapabilityTest, InvalidPropertiesUseProvidedNodeException) {
    TestRendererCapability renderer;
    const PropertyMap missing_property;
    const PropertyMap wrong_property_type = {
        {kNameProperty, Property{.type_id = 3, .value = 42}},
    };

    EXPECT_THROW(
        renderer.GetNodeComponents(1, kSupportedNodeType, missing_property),
        core::InvalidNodeException);
    EXPECT_THROW(
        renderer.GetNodeComponents(1, kSupportedNodeType, wrong_property_type),
        core::InvalidNodeException);
}

TEST(RendererCapabilityTest, CanBeDestroyedPolymorphically) {
    std::unique_ptr<IRendererCapability> renderer =
        std::make_unique<TestRendererCapability>();

    EXPECT_TRUE(renderer->SupportsNodeType(kSupportedNodeType));
}

}  // namespace
