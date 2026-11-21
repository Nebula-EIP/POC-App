/**
 * @file graph_validation.cpp
 * @brief Implementation of editor graph validation display and node/pin error
 * highlighting.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-11-2026
 */

#include "ui/graph_validation.hpp"

namespace editor::ui {

void HighlightValidationErrors(NodeCanvas &canvas,
                               const core::capa::ValidationResult &result) {
    const auto &views = canvas.Views();
    for (const auto &[kId, kView] : views) {
        (void)kId;
        kView->ClearErrorPin();
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

core::capa::ValidationResult ValidateGraphForCodegen(
    const core::Graph &graph, NodeCanvas &canvas,
    const core::capa::IValidationCapability &validator) {
    const core::capa::ValidationResult kResult = validator.Validate(graph);
    HighlightValidationErrors(canvas, kResult);
    return kResult;
}

}  // namespace editor::ui
