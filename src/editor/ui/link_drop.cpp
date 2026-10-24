/**
 * @file link_drop.cpp
 * @brief Implementation of the link drop helpers.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 24-10-2026
 */

#include "link_drop.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace editor::ui {

namespace {

/// Between the node name and the pin name in a label. The picker draws with
/// raylib's default font, which only has ASCII and Latin-1 glyphs.
constexpr const char *kPinSeparator = " » ";

HitPart PartOf(bool is_input) noexcept {
    return is_input ? HitPart::kInputPin : HitPart::kOutputPin;
}

std::string TypeText(core::DataType type, const TypeNameResolver &type_name) {
    std::string name = type_name ? type_name(type) : std::string{};
    return name.empty() ? "type " + std::to_string(type) : name;
}

}  // namespace

std::optional<DraggedPin> FindDraggedPin(const core::Graph &graph,
                                         const core::NodeCatalog &catalog,
                                         const HitResult &pin) {
    const core::Node *node = graph.GetNode(pin.node_id_);
    if (node == nullptr) {
        return std::nullopt;
    }
    const core::Pin *found =
        pin.part_ == HitPart::kInputPin    ? node->InputPin(pin.pin_id_)
        : pin.part_ == HitPart::kOutputPin ? node->OutputPin(pin.pin_id_)
                                           : nullptr;
    const std::optional<core::NodeDescriptor> kDescriptor =
        catalog.GetNode(node->Type());
    if (found == nullptr || !kDescriptor) {
        return std::nullopt;
    }
    return DraggedPin{.pin_ = pin,
                      .type_ = found->type_,
                      .module_id_ = kDescriptor->module_id_};
}

std::vector<NodePickerEntry> LinkDropEntries(
    const core::NodeCatalog &catalog, const DraggedPin &dragged,
    const std::function<std::string(core::ModuleId)> &module_name,
    const TypeNameResolver &type_name) {
    const PinEnd kStart{.node_ = dragged.pin_.node_id_,
                        .part_ = dragged.pin_.part_,
                        .type_ = dragged.type_};

    std::vector<NodePickerEntry> entries;
    for (const core::NodeDescriptor &node : catalog.GetNodes()) {
        if (node.module_id_ != dragged.module_id_) {
            continue;
        }
        std::size_t input_index = 0;
        std::size_t output_index = 0;
        for (const core::NodePinDescriptor &pin : node.pins_) {
            // Counted before the check: the index is the position of the pin
            // among all the pins of its direction.
            const std::size_t kIndex =
                pin.is_input_ ? input_index++ : output_index++;
            // The node does not exist yet: it is never the dragged node.
            const PinEnd kEnd{.node_ = std::nullopt,
                              .part_ = PartOf(pin.is_input_),
                              .type_ = pin.type_};
            if (CheckConnectionRule(kStart, kEnd) != ConnectionRule::kAllowed) {
                continue;
            }
            NodePickerEntry entry;
            entry.type_ = node.id_;
            entry.name_ = node.name_ + kPinSeparator + pin.name_ + " (" +
                          TypeText(pin.type_, type_name) + ")";
            entry.module_ =
                module_name ? module_name(node.module_id_) : std::string{};
            entry.category_ = node.category_;
            entry.pin_ = PickerPin{.is_input_ = pin.is_input_,
                                   .index_ = kIndex,
                                   .name_ = pin.name_,
                                   .type_ = pin.type_};
            entry.search_names_ = {node.name_, pin.name_};
            entries.push_back(std::move(entry));
        }
    }
    return entries;
}

std::optional<HitResult> ResolvePickerPin(const core::Node &node,
                                          const PickerPin &pin) {
    const std::vector<core::Pin> &pins =
        pin.is_input_ ? node.InputPins() : node.OutputPins();
    if (pin.index_ >= pins.size()) {
        return std::nullopt;
    }
    const core::Pin &found = pins[pin.index_];
    if (found.name_ != pin.name_ || found.type_ != pin.type_) {
        return std::nullopt;
    }
    return HitResult{.node_id_ = node.Id(),
                     .pin_id_ = found.id_,
                     .part_ = PartOf(pin.is_input_)};
}

ConnectionOutcome LinkNewNode(core::Graph &graph, const HitResult &dragged,
                              core::NodeId new_node, const PickerPin &pin,
                              const TypeNameResolver &type_name) {
    ConnectionOutcome outcome;
    const core::Node *node = graph.GetNode(new_node);
    const std::optional<HitResult> kTarget =
        node != nullptr ? ResolvePickerPin(*node, pin) : std::nullopt;
    if (!kTarget) {
        outcome.error_ = "Connexion impossible : la broche « " + pin.name_ +
                         " » est introuvable sur le nouveau node.";
    } else {
        // Same order as a manual drag: from the dragged pin to the new one.
        outcome = RequestConnection(graph, dragged, *kTarget, type_name);
        if (outcome.Succeeded()) {
            return outcome;
        }
        if (outcome.error_.empty()) {
            outcome.error_ = "Connexion impossible.";
        }
    }
    outcome.faulty_pin_ = dragged;
    graph.RemoveNode(new_node);
    return outcome;
}

}  // namespace editor::ui
