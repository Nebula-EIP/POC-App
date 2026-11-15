/**
 * @file graph_validation.hpp
 * @brief Runs graph validation for C++ code generation and highlights affected
 * nodes and pins in the editor canvas.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 15-11-2026
 */

#pragma once

#include "graph/graph.hpp"
#include "graph/graph_validator.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"
#include "node_catalog/node_catalog.hpp"
#include "ui/node_canvas.hpp"

namespace editor::ui {

/**
 * @brief Clears previous error highlights on the canvas and highlights all
 * nodes and pins involved in the given validation result.
 *
 * @param canvas The editor node canvas whose node views will be updated.
 * @param result The graph validation result to visualize.
 */
void HighlightValidationErrors(
    NodeCanvas &canvas, const core::GraphValidator::ValidationResult &result);

/**
 * @brief Validates the graph before C++ export or compilation and highlights
 * any affected nodes and pins on the canvas.
 *
 * @param graph The graph to validate.
 * @param canvas The node canvas used to resolve node titles and highlight
 * errors.
 * @param catalog Optional node catalog of loaded modules.
 * @param nodes Optional node list capability of the active module.
 * @param types Optional type list capability of the active module.
 * @return The complete validation result; when `is_valid_` is false, export and
 * compilation must stop.
 */
[[nodiscard]] core::GraphValidator::ValidationResult ValidateGraphForCodegen(
    const core::Graph &graph, NodeCanvas &canvas,
    const core::NodeCatalog *catalog = nullptr,
    const core::capa::INodeListCapability *nodes = nullptr,
    const core::capa::ITypeListCapability *types = nullptr);

}  // namespace editor::ui
