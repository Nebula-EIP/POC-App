/**
 * @file graph_validation.cpp
 * @brief Implementation of editor graph validation and node/pin error
 * highlighting.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 15-11-2026
 */

#include "ui/graph_validation.hpp"

#include <string>

namespace editor::ui {

void HighlightValidationErrors(
    NodeCanvas &canvas, const core::GraphValidator::ValidationResult &result) {
    const auto &views = canvas.Views();
    for (const auto &[id, view] : views) {
        (void)id;
        view->ClearErrorPin();
    }

    if (result.is_valid_) {
        return;
    }

    for (const auto &error : result.errors_) {
        for (const core::NodeId kNodeId : error.involved_node_ids_) {
            const auto kViewIt = views.find(kNodeId);
            if (kViewIt != views.end()) {
                kViewIt->second->SetHasError(true);
            }
        }
        for (const auto &pin : error.involved_pins_) {
            const auto kViewIt = views.find(pin.node_id_);
            if (kViewIt != views.end() && kViewIt->second->ErrorPinId() == 0) {
                kViewIt->second->SetErrorPin(pin.pin_id_, pin.is_input_);
            }
        }
    }
}

core::GraphValidator::ValidationResult ValidateGraphForCodegen(
    const core::Graph &graph, NodeCanvas &canvas,
    const core::NodeCatalog *catalog,
    const core::capa::INodeListCapability *nodes,
    const core::capa::ITypeListCapability *types) {
    core::GraphValidator::ValidationContext context;
    context.catalog_ = catalog;
    context.nodes_ = nodes;
    context.types_ = types;
    context.node_title_ = [&canvas](core::NodeId node_id) -> std::string {
        const auto &views = canvas.Views();
        const auto kViewIt = views.find(node_id);
        if (kViewIt == views.end()) {
            return {};
        }
        return kViewIt->second->Title();
    };

    const core::GraphValidator kValidator;
    const auto kResult = kValidator.Validate(graph, context);
    HighlightValidationErrors(canvas, kResult);
    return kResult;
}

}  // namespace editor::ui
