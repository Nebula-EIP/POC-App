/**
 * @file graph_validation.hpp
 * @brief Runs graph validation through a module validation capability and
 * highlights affected nodes and pins in the editor canvas.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-11-2026
 */

#pragma once

#include "graph/graph.hpp"
#include "modules/capabilities/validation_capability.hpp"
#include "ui/node_canvas.hpp"

namespace editor::ui {

/**
 * @brief Clears previous error highlights on the canvas and highlights all
 * nodes and pins involved in the given validation result.
 *
 * @param canvas The editor node canvas whose node views will be updated.
 * @param result The graph validation result to visualize.
 */
void HighlightValidationErrors(NodeCanvas &canvas,
                               const core::capa::ValidationResult &result);

/**
 * @brief Validates the graph using the provided module validation capability
 * and highlights any affected nodes and pins on the canvas.
 *
 * @param graph The graph to validate.
 * @param canvas The node canvas used to highlight affected nodes and pins.
 * @param validator The validation capability of the active module.
 * @return The complete validation result; when `is_valid_` is false, export and
 * compilation must stop.
 */
[[nodiscard]] core::capa::ValidationResult ValidateGraphForCodegen(
    const core::Graph &graph, NodeCanvas &canvas,
    const core::capa::IValidationCapability &validator);

}  // namespace editor::ui
