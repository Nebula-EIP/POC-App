/**
 * @file connection_request.hpp
 * @brief Turns a pin-to-pin drag on the canvas into a graph connection, or
 * into an error message the user can understand
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#pragma once

#include <functional>
#include <optional>
#include <string>

#include "graph/graph.hpp"
#include "node_view.hpp"

namespace editor::ui {

/// Returns the display name of a data type (empty when unknown).
using TypeNameResolver = std::function<std::string(core::DataType)>;

/**
 * @brief Result of a connection request.
 *
 * On success connection_ holds the new (or already existing) connection and
 * error_ is empty. On failure connection_ is empty, error_ explains why, and
 * faulty_pin_ is the pin to highlight. When the drag ends on the pin it
 * started from, nothing happens: both connection_ and error_ are empty.
 */
struct ConnectionOutcome {
    std::optional<core::ConnectionId> connection_;
    std::string error_;
    std::optional<HitResult> faulty_pin_;

    [[nodiscard]] bool Succeeded() const noexcept {
        return connection_.has_value();
    }
};

/**
 * @brief Connects the two pins of a drag, in whichever order they were
 * picked (output to input, or input to output).
 *
 * Rejected with a message: two inputs, two outputs, a node linked to itself,
 * mismatching types, and any other error raised by core::Graph::Connect.
 *
 * @param graph The graph to connect in.
 * @param start The pin the drag started from.
 * @param end The pin the drag was released on.
 * @param type_name Resolves type names for the mismatch message. May be
 * empty, in which case types are shown by id.
 *
 * @return The outcome; the graph is left untouched on failure.
 */
[[nodiscard]] ConnectionOutcome RequestConnection(
    core::Graph &graph, const HitResult &start, const HitResult &end,
    const TypeNameResolver &type_name = {});

}  // namespace editor::ui
