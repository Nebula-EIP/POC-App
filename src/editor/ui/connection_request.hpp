/**
 * @file connection_request.hpp
 * @brief Turns a pin-to-pin drag on the canvas into a graph connection, or
 * into an error message the user can understand
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 24-10-2026
 */

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "graph/graph.hpp"
#include "node_view.hpp"

namespace editor::ui {

/// Returns the display name of a data type (empty when unknown).
using TypeNameResolver = std::function<std::string(core::DataType)>;

/**
 * @brief One end of a possible connection, as the rules see it.
 */
struct PinEnd {
    /// Node of the pin. Empty for a node that does not exist yet (a node the
    /// picker could create): it is never the node of the other end.
    std::optional<core::NodeId> node_;
    HitPart part_ = HitPart::kNone;
    /// Type of the pin. Empty when unknown: the type rule is then skipped.
    std::optional<core::DataType> type_;
};

/// Rule a connection between two pins breaks, or kAllowed.
enum class ConnectionRule : std::uint8_t {
    kAllowed,
    kNotAPin,        ///< One end is not a pin.
    kSameDirection,  ///< Two inputs or two outputs.
    kSameNode,       ///< A node linked to itself.
    kTypeMismatch,   ///< The pins do not carry the same type.
};

/**
 * @brief The rules of a connection, shared by RequestConnection and the node
 * picker opened from a link dropped on empty canvas, so that the picker never
 * offers a connection that would then be refused.
 *
 * Checked in this order: both ends are pins, opposite directions, two
 * different nodes, same type.
 */
[[nodiscard]] ConnectionRule CheckConnectionRule(const PinEnd &start,
                                                 const PinEnd &end) noexcept;

/**
 * @brief Result of a connection request.
 *
 * On success connection_ holds the new (or already existing) connection and
 * error_ is empty. On failure connection_ is empty, error_ explains why, and
 * faulty_pin_ is the pin to highlight. When the drag ends on the pin it
 * started from, nothing happens: both connection_ and error_ are empty.
 * replaced_ lists the connections removed because the new one took their
 * input.
 */
struct ConnectionOutcome {
    std::optional<core::ConnectionId> connection_;
    std::string error_;
    std::optional<HitResult> faulty_pin_;
    std::vector<core::ConnectionId> replaced_;

    [[nodiscard]] bool Succeeded() const noexcept;
};

/**
 * @brief Connects the two pins of a drag, in whichever order they were
 * picked (output to input, or input to output).
 *
 * An input accepts a single connection: the new connection takes priority
 * and replaces the one already on the input (an output can still feed
 * several inputs).
 *
 * Rejected with a message when CheckConnectionRule refuses the pins (two
 * inputs, two outputs, a node linked to itself, mismatching types), and on
 * any other error raised by core::Graph::Connect.
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
