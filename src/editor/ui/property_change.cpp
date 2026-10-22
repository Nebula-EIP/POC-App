/**
 * @file property_change.cpp
 * @brief Applies the property changes made in a node's fields to the graph.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 11-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#include "property_change.hpp"

#include <algorithm>
#include <exception>
#include <string>
#include <vector>

namespace editor::ui {

namespace {

void CollectChanges(const std::vector<core::Pin> &current,
                    const std::vector<core::Pin> &wanted, bool is_input,
                    std::vector<PinTypeChange> &changes) {
    const std::size_t kCount = std::min(current.size(), wanted.size());
    for (std::size_t index = 0; index < kCount; ++index) {
        if (current[index].type_ != wanted[index].type_) {
            changes.push_back(PinTypeChange{
                .pin_id_ = current[index].id_,
                .is_input_ = is_input,
                .from_ = current[index].type_,
                .to_ = wanted[index].type_,
            });
        }
    }
}

std::string PinName(const core::Node &node, const PinTypeChange &change) {
    const core::Pin *pin = change.is_input_ ? node.InputPin(change.pin_id_)
                                            : node.OutputPin(change.pin_id_);
    return pin == nullptr ? std::string{"?"} : pin->name_;
}

}  // namespace

PropertyChangeOutcome ApplyPropertyChange(
    core::Graph &graph, const core::capa::INodeListCapability *nodes,
    core::NodeId node_id, const core::PropertyMap &changes,
    const std::function<std::string(core::DataType)> &type_name,
    const std::function<std::string(core::NodeId)> &node_title) {
    PropertyChangeOutcome outcome;
    core::Node *node = graph.GetNode(node_id);
    if (node == nullptr) {
        outcome.error_ = "Ce nœud n'existe plus.";
        return outcome;
    }
    if (changes.empty()) {
        outcome.applied_ = true;
        return outcome;
    }

    core::PropertyMap wanted = node->Properties();
    for (const auto &[id, property] : changes) {
        wanted.insert_or_assign(id, property);
    }

    if (nodes != nullptr) {
        try {
            const core::capa::NodeConfiguration kConfig =
                nodes->ResolveNodeConfiguration(node->Type(), wanted);
            CollectChanges(node->InputPins(), kConfig.input_pins_, true,
                           outcome.pin_changes_);
            CollectChanges(node->OutputPins(), kConfig.output_pins_, false,
                           outcome.pin_changes_);
        } catch (const std::exception &) {
            // Unknown to the module: its pins do not depend on properties.
            outcome.pin_changes_.clear();
        }
    }

    // A connected pin keeps its type: the whole change is refused.
    for (const PinTypeChange &change : outcome.pin_changes_) {
        const auto kConnections =
            graph.PinConnections(node_id, change.pin_id_, change.is_input_);
        if (kConnections.empty()) {
            continue;
        }
        if (!outcome.blocked_pin_) {
            outcome.blocked_pin_ = change;
        }
        outcome.blocking_connections_.insert(
            outcome.blocking_connections_.end(), kConnections.begin(),
            kConnections.end());
    }
    if (outcome.blocked_pin_) {
        const PinTypeChange &pin = *outcome.blocked_pin_;
        std::vector<core::NodeId> others;
        for (const core::ConnectionId kId : outcome.blocking_connections_) {
            const core::Connection &connection =
                graph.GetAllConnections().at(kId);
            const core::NodeId kOther = connection.from_node_ == node_id
                                            ? connection.to_node_
                                            : connection.from_node_;
            if (std::find(others.begin(), others.end(), kOther) ==
                others.end()) {
                others.push_back(kOther);
            }
        }
        std::string names;
        for (std::size_t index = 0; index < others.size(); ++index) {
            if (index > 0) names += ", ";
            names += "« " + node_title(others[index]) + " »";
        }
        const bool kSeveral = outcome.blocking_connections_.size() > 1;
        outcome.error_ = std::string{"Impossible de passer "} +
                         (pin.is_input_ ? "l'entrée" : "la sortie") + " « " +
                         PinName(*node, pin) + " » de " + type_name(pin.from_) +
                         " à " + type_name(pin.to_) + " : elle est reliée à " +
                         names + ". Supprimez d'abord " +
                         (kSeveral ? "ces liaisons." : "cette liaison.");
        return outcome;
    }

    for (const auto &[id, property] : changes) {
        node->SetProperty(id, property);
    }
    for (const PinTypeChange &change : outcome.pin_changes_) {
        graph.SetPinType(node_id, change.pin_id_, change.is_input_, change.to_);
    }
    outcome.applied_ = true;
    return outcome;
}

}  // namespace editor::ui
