/**
 * @file renderer_capability.hpp
 * @brief Defines the UI components that can be provided by a renderer capability.
 *
 * This file provides generic UI component descriptions that modules can use
 * to describe how their nodes should be displayed and interacted with.
 *
 * The components defined here must not perform any rendering themselves.
 * Rendering is handled by the application's renderer.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 10-08-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 10-08-2026
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

#include "../../graph/datatypes.hpp"
#include "../icapability.hpp"

namespace core {

namespace capa {

/**
 * @brief Unique identifier for a UI component.
 */
using ComponentId = uint16_t;

/**
 * @brief Base information shared by every UI component.
 */
struct ComponentBase {
    /** @brief Unique identifier of the component. */
    ComponentId id = 0;

    /** @brief Optional contextual information displayed by the renderer. */
    std::string tooltip;
};

// ============================================================================
// LABEL
// ============================================================================

/**
 * @brief Simple static text component.
 */
struct Label : public ComponentBase {
    /** @brief Text displayed by the label. */
    std::string text;
};

// ============================================================================
// BUTTON
// ============================================================================

/**
 * @brief Clickable button component.
 */
struct Button : public ComponentBase {
    /** @brief Text displayed inside the button. */
    std::string label;

    /** @brief Indicates whether the button can currently be interacted with. */
    bool enabled = true;

    /**
     * @brief Callback executed when the button is clicked.
     */
    std::function<void()> on_click;
};

// ============================================================================
// TEXT FIELD
// ============================================================================

/**
 * @brief Editable text field component.
 */
struct TextField : public ComponentBase {
    /** @brief Label displayed next to the text field. */
    std::string label;

    /** @brief Current value of the text field. */
    std::string value;

    /** @brief Indicates whether the text field can only be read. */
    bool read_only = false;

    /** @brief Indicates whether the text field can currently be interacted with. */
    bool enabled = true;

    /**
     * @brief Callback executed when the text field value changes.
     *
     * @param value New value entered by the user.
     */
    std::function<void(const std::string &value)> on_change;
};

// ============================================================================
// NUMBER FIELD
// ============================================================================

/**
 * @brief Editable numeric field component.
 */
struct NumberField : public ComponentBase {
    /** @brief Label displayed next to the numeric field. */
    std::string label;

    /** @brief Current numeric value. */
    double value = 0.0;

    /** @brief Minimum accepted value. */
    double min = 0.0;

    /** @brief Maximum accepted value. */
    double max = 1.0;

    /** @brief Step used when increasing or decreasing the value. */
    double step = 0.0;

    /** @brief Indicates whether only integer values are accepted. */
    bool is_integer = false;

    /** @brief Indicates whether the field can currently be interacted with. */
    bool enabled = true;

    /**
     * @brief Callback executed when the numeric value changes.
     *
     * @param value New value entered by the user.
     */
    std::function<void(double value)> on_change;
};

// ============================================================================
// SLIDER
// ============================================================================

/**
 * @brief Numeric slider component.
 */
struct Slider : public ComponentBase {
    /** @brief Label displayed next to the slider. */
    std::string label;

    /** @brief Current value of the slider. */
    double value = 0.0;

    /** @brief Minimum value accepted by the slider. */
    double min = 0.0;

    /** @brief Maximum value accepted by the slider. */
    double max = 1.0;

    /** @brief Step used when moving the slider. */
    double step = 0.0;

    /** @brief Indicates whether the slider accepts only integer values. */
    bool is_integer = false;

    /** @brief Indicates whether the slider can currently be interacted with. */
    bool enabled = true;

    /**
     * @brief Callback executed when the slider value changes.
     *
     * @param value New value selected by the user.
     */
    std::function<void(double value)> on_change;
};

// ============================================================================
// CHECKBOX
// ============================================================================

/**
 * @brief Boolean checkbox component.
 */
struct Checkbox : public ComponentBase {
    /** @brief Label displayed next to the checkbox. */
    std::string label;

    /** @brief Current state of the checkbox. */
    bool value = false;

    /** @brief Indicates whether the checkbox can currently be interacted with. */
    bool enabled = true;

    /**
     * @brief Callback executed when the checkbox state changes.
     *
     * @param value New checkbox state.
     */
    std::function<void(bool value)> on_change;
};

// ============================================================================
// SELECT
// ============================================================================

/**
 * @brief Component allowing the user to select one value from a list.
 */
struct Select : public ComponentBase {
    /** @brief Label displayed next to the select component. */
    std::string label;

    /** @brief Available options. */
    std::vector<std::string> options;

    /** @brief Index of the currently selected option. */
    std::size_t selected_index = 0;

    /** @brief Indicates whether the select can currently be interacted with. */
    bool enabled = true;

    /**
     * @brief Callback executed when the selected option changes.
     *
     * @param index Index of the newly selected option.
     */
    std::function<void(std::size_t index)> on_change;
};

// ============================================================================
// INFO BUBBLE
// ============================================================================

/**
 * @brief Informational text component.
 *
 * Can be used by the renderer to display contextual information,
 * help messages, warnings, or information bubbles.
 */
struct InfoBubble : public ComponentBase {
    /** @brief Information displayed by the component. */
    std::string text;
};

// ============================================================================
// SEPARATOR
// ============================================================================

/**
 * @brief Visual separator between groups of components.
 *
 * The actual representation of the separator is decided by the renderer.
 */
struct Separator : public ComponentBase {
};

// ============================================================================
// COMPONENT
// ============================================================================

/**
 * @brief Represents any UI component supported by the renderer.
 *
 * The renderer can use std::visit to identify the concrete component type
 * and display it appropriately.
 */
using Component = std::variant<
    Label,
    Button,
    TextField,
    NumberField,
    Slider,
    Checkbox,
    Select,
    InfoBubble,
    Separator>;

/**
 * @brief List of components describing a node's user interface.
 */
using ComponentList = std::vector<Component>;

// ============================================================================
// RENDERER CAPABILITY
// ============================================================================

/**
 * @brief Interface for the capability providing custom node rendering.
 *
 * This capability allows a module to describe how its nodes should be
 * displayed by providing UI components to the application's renderer.
 *
 * The capability must not perform any rendering itself. It only describes
 * the components to display and provides the callbacks associated with
 * user interactions.
 *
 * This capability is optional for modules. Nodes that do not provide custom
 * rendering can be displayed using the application's default renderer.
 */
class IRendererCapability : public ICapability {
   public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~IRendererCapability() = default;

    /**
     * @brief Checks whether custom rendering is provided for a node type.
     *
     * @param node_type Type of the node to check.
     *
     * @return true if this capability provides custom UI for the node type,
     * false otherwise.
     */
    virtual bool SupportsNodeType(NodeType node_type) const noexcept = 0;

    /**
     * @brief Retrieves the UI components used to display a specific node.
     *
     * The returned components describe how the node should be displayed
     * and interacted with. No actual rendering must be performed by this
     * function.
     *
     * @param node_id Unique identifier of the node instance.
     * @param node_type Type of the node.
     * @param properties Current properties of the node instance.
     *
     * @return List of components describing the node's custom interface.
     */
    virtual ComponentList GetNodeComponents(
        NodeId node_id,
        NodeType node_type,
        const PropertyMap &properties) = 0;
};

}  // namespace capa

}  // namespace core