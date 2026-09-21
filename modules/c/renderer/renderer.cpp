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

#include "renderer.hpp"

#include <algorithm>
#include <any>
#include <string>
#include <utility>

namespace {

namespace c_properties = c_module::renderer::properties;
using c_module::renderer::PropertyUpdate;
using core::Property;
using core::PropertyMap;
using core::capa::ComponentList;

template <typename T>
const T *Read(const PropertyMap &properties, core::PropertyId id) {
    const auto kProperty = properties.find(id);
    if (kProperty == properties.end()) return nullptr;
    return std::any_cast<T>(&kProperty->second.value_);
}

core::capa::InfoBubble InvalidProperty(core::PropertyId id,
                                       const char *expected) {
    core::capa::InfoBubble message;
    message.text_ = "Property " + std::to_string(id) +
                    " is missing or has an invalid type (expected " + expected +
                    ")";
    return message;
}

template <typename T>
void Set(core::NodeId node_id, core::PropertyId property_id, T value,
         const PropertyUpdate &update) {
    if (!update) return;
    update(node_id, property_id,
           Property{.type_id_ = property_id, .value_ = std::move(value)});
}

core::capa::Label Title(const char *text) {
    core::capa::Label title;
    title.id_ = 1;
    title.text_ = text;
    return title;
}

ComponentList Declaration(core::NodeId node_id, core::NodeType,
                          const PropertyMap &properties,
                          const PropertyUpdate &update) {
    ComponentList components{Title("Declaration")};
    const auto kName = Read<std::string>(properties, c_properties::kName);
    if (kName == nullptr) {
        components.emplace_back(InvalidProperty(c_properties::kName, "string"));
        return components;
    }
    core::capa::TextField field;
    field.id_ = 2;
    field.label_ = "Name";
    field.value_ = *kName;
    field.on_change_ = [node_id, update](const std::string &value) {
        Set(node_id, c_properties::kName, value, update);
    };
    components.emplace_back(std::move(field));
    return components;
}

ComponentList Literal(core::NodeId node_id, core::NodeType,
                      const PropertyMap &properties,
                      const PropertyUpdate &update) {
    ComponentList components{Title("Literal")};
    const auto kValue = Read<double>(properties, c_properties::kValue);
    if (kValue == nullptr) {
        components.emplace_back(
            InvalidProperty(c_properties::kValue, "number"));
        return components;
    }
    core::capa::NumberField field;
    field.id_ = 2;
    field.label_ = "Value";
    field.value_ = *kValue;
    field.min_ = -1000000.0;
    field.max_ = 1000000.0;
    field.step_ = 1.0;
    field.on_change_ = [node_id, update](double changed) {
        Set(node_id, c_properties::kValue, changed, update);
    };
    components.emplace_back(std::move(field));
    return components;
}

ComponentList BinaryExpression(core::NodeId node_id, core::NodeType,
                               const PropertyMap &properties,
                               const PropertyUpdate &update) {
    ComponentList components{Title("Binary expression")};
    const auto kOperation =
        Read<std::string>(properties, c_properties::kOperator);
    if (kOperation == nullptr) {
        components.emplace_back(
            InvalidProperty(c_properties::kOperator, "string"));
        return components;
    }
    core::capa::Select select;
    select.id_ = 2;
    select.label_ = "Operator";
    select.options_ = {"+", "-", "*", "/"};
    const auto kPosition =
        std::find(select.options_.begin(), select.options_.end(), *kOperation);
    select.selected_index_ =
        kPosition == select.options_.end()
            ? 0U
            : static_cast<std::size_t>(kPosition - select.options_.begin());
    select.on_change_ = [node_id, update](std::size_t index) {
        static constexpr const char *kOperators[] = {"+", "-", "*", "/"};
        if (index < 4) {
            Set(node_id, c_properties::kOperator,
                std::string{kOperators[index]}, update);
        }
    };
    components.emplace_back(std::move(select));
    return components;
}

ComponentList Conditional(core::NodeId node_id, core::NodeType,
                          const PropertyMap &properties,
                          const PropertyUpdate &update) {
    ComponentList components{Title("Conditional")};
    const auto kCondition = Read<bool>(properties, c_properties::kCondition);
    if (kCondition == nullptr) {
        components.emplace_back(
            InvalidProperty(c_properties::kCondition, "bool"));
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

}  // namespace

namespace c_module::renderer {

void RegisterProviders(core::capa::RendererCapability &renderer,
                       PropertyUpdate update) {
    renderer.RegisterNodeRenderer(node_types::kDeclaration,
                                  [update](auto id, auto type, const auto &p) {
                                      return Declaration(id, type, p, update);
                                  });
    renderer.RegisterNodeRenderer(node_types::kLiteral,
                                  [update](auto id, auto type, const auto &p) {
                                      return Literal(id, type, p, update);
                                  });
    renderer.RegisterNodeRenderer(node_types::kBinaryExpression,
                                  [update](auto id, auto type, const auto &p) {
                                      return BinaryExpression(id, type, p,
                                                              update);
                                  });
    renderer.RegisterNodeRenderer(node_types::kConditional,
                                  [update](auto id, auto type, const auto &p) {
                                      return Conditional(id, type, p, update);
                                  });
    renderer.RegisterNodeRenderer(node_types::kLoop,
                                  [update](auto id, auto type, const auto &p) {
                                      return Conditional(id, type, p, update);
                                  });
}

}  // namespace c_module::renderer
