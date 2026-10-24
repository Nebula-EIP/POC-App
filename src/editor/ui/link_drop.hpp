/**
 * @file link_drop.hpp
 * @brief A link dragged from a pin and released on empty canvas: the nodes
 * that can be linked to the pin, offered by the node picker, and the link to
 * the picked node once it is created.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 24-10-2026
 */

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "connection_request.hpp"
#include "graph/graph.hpp"
#include "node_catalog/node_catalog.hpp"
#include "node_picker.hpp"
#include "node_view.hpp"

namespace editor::ui {

/**
 * @brief The pin a link was dragged from, with what the candidates must
 * match.
 */
struct DraggedPin {
    HitResult pin_;
    core::DataType type_ = 0;
    /// Module of the dragged node. Type ids are given per module, so only the
    /// nodes of this module are offered.
    core::ModuleId module_id_ = 0;
};

/**
 * @brief Finds the type and the module of a dragged pin.
 *
 * @return Empty when the node, the pin or its catalog entry is missing.
 */
[[nodiscard]] std::optional<DraggedPin> FindDraggedPin(
    const core::Graph &graph, const core::NodeCatalog &catalog,
    const HitResult &pin);

/**
 * @brief Lists the pins of the catalog nodes that can be linked to the
 * dragged pin, one entry per pin, labelled "Node › pin (type)".
 *
 * A pin fits when CheckConnectionRule accepts it (opposite direction, same
 * type) and its node comes from the module of the dragged node. Entries keep
 * the catalog order, then the order of the pins in the node.
 *
 * @param module_name Returns the displayed name of a module (top folder).
 * @param type_name Returns the name of a type of the dragged node's module.
 * May be empty, or return an empty name: the type is then shown by id.
 */
[[nodiscard]] std::vector<NodePickerEntry> LinkDropEntries(
    const core::NodeCatalog &catalog, const DraggedPin &dragged,
    const std::function<std::string(core::ModuleId)> &module_name,
    const TypeNameResolver &type_name = {});

/**
 * @brief Finds, on a node just created, the pin a picker entry stands for:
 * the pin at the same position among the pins of the same direction, which
 * must have the same name and type.
 *
 * @return Empty when the node has no such pin.
 */
[[nodiscard]] std::optional<HitResult> ResolvePickerPin(const core::Node &node,
                                                        const PickerPin &pin);

/**
 * @brief Links the dragged pin to the picked pin of a node just created,
 * through RequestConnection so that the usual rules and messages apply.
 *
 * When the link is refused, the new node is removed from the graph (the
 * caller removes its view), so that no unlinked node is left behind.
 *
 * @param new_node The node created from the picked entry.
 * @param pin The picked pin of that node.
 *
 * @return The outcome of the connection request. On failure the faulty pin
 * is the dragged one, since the new node is gone.
 */
[[nodiscard]] ConnectionOutcome LinkNewNode(
    core::Graph &graph, const HitResult &dragged, core::NodeId new_node,
    const PickerPin &pin, const TypeNameResolver &type_name = {});

}  // namespace editor::ui
