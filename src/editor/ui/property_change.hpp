/**
 * @file property_change.hpp
 * @brief Applies the property changes made in a node's fields to the graph.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "core/graph/graph.hpp"
#include "core/modules/capabilities/node_list_capability.hpp"

namespace editor::ui {

/// A pin whose type changes with the properties.
struct PinTypeChange {
    core::PinId pin_id_ = 0;
    bool is_input_ = false;
    core::DataType from_ = 0;
    core::DataType to_ = 0;
};

/// Result of a property change request.
struct PropertyChangeOutcome {
    bool applied_ = false;
    std::vector<PinTypeChange> pin_changes_;  ///< Applied (or refused) changes
    /// Connections that prevent the change (empty when applied).
    std::vector<core::ConnectionId> blocking_connections_;
    /// Pin of the edited node that is still connected, when refused.
    std::optional<PinTypeChange> blocked_pin_;
    std::string error_;  ///< French message when refused
};

/**
 * @brief Applies new property values to a node of the graph.
 *
 * The node list capability tells which pin types the new properties give
 * (a literal switched to float outputs a float). A connected pin never
 * changes type: the whole change is refused, nothing is modified, and the
 * outcome lists the connections to remove first.
 *
 * @param graph The graph holding the node.
 * @param nodes Node list capability of the node's module (may be null: pins
 * then never change).
 * @param node_id The edited node.
 * @param changes Properties to set, applied together.
 * @param type_name Name of a data type, for the messages.
 * @param node_title Title of a node, for the messages.
 */
PropertyChangeOutcome ApplyPropertyChange(
    core::Graph &graph, const core::capa::INodeListCapability *nodes,
    core::NodeId node_id, const core::PropertyMap &changes,
    const std::function<std::string(core::DataType)> &type_name,
    const std::function<std::string(core::NodeId)> &node_title);

}  // namespace editor::ui
