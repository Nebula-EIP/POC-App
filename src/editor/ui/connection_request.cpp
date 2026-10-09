/**
 * @file connection_request.cpp
 * @brief Implementation of the canvas connection request
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "connection_request.hpp"

#include <exception>
#include <string>
#include <utility>

#include "exception/graph_exception/connection_exception.hpp"

namespace editor::ui {

namespace {

bool IsPin(const HitResult &hit) noexcept {
    return hit.part_ == HitPart::kInputPin || hit.part_ == HitPart::kOutputPin;
}

ConnectionOutcome Failure(std::string message, const HitResult &pin) {
    return ConnectionOutcome{
        .connection_ = std::nullopt,
        .error_ = std::move(message),
        .faulty_pin_ = pin,
    };
}

std::string TypeLabel(core::DataType type, const TypeNameResolver &type_name) {
    std::string name = type_name ? type_name(type) : std::string{};
    if (name.empty()) {
        name = "type " + std::to_string(type);
    }
    return "« " + name + " »";
}

}  // namespace

ConnectionOutcome RequestConnection(core::Graph &graph, const HitResult &start,
                                    const HitResult &end,
                                    const TypeNameResolver &type_name) {
    // Released on the pin it started from: a click, not a connection.
    if (start.node_id_ == end.node_id_ && start.pin_id_ == end.pin_id_ &&
        start.part_ == end.part_) {
        return {};
    }
    if (!IsPin(start) || !IsPin(end)) {
        return Failure("Relâchez le lien sur une entrée ou une sortie.", end);
    }
    if (start.part_ == end.part_) {
        return Failure(start.part_ == HitPart::kInputPin
                           ? "Impossible de relier deux entrées : reliez une "
                             "sortie à une entrée."
                           : "Impossible de relier deux sorties : reliez une "
                             "sortie à une entrée.",
                       end);
    }
    if (start.node_id_ == end.node_id_) {
        return Failure("Un node ne peut pas être relié à lui-même.", end);
    }

    const HitResult &output = start.part_ == HitPart::kOutputPin ? start : end;
    const HitResult &input = start.part_ == HitPart::kInputPin ? start : end;

    try {
        return ConnectionOutcome{
            .connection_ = graph.Connect(output.node_id_, output.pin_id_,
                                         input.node_id_, input.pin_id_),
            .error_ = {},
            .faulty_pin_ = std::nullopt,
        };
    } catch (const core::TypeMismatchException &) {
        const core::Node *from = graph.GetNode(output.node_id_);
        const core::Node *to = graph.GetNode(input.node_id_);
        const core::Pin *out_pin = from->OutputPin(output.pin_id_);
        const core::Pin *in_pin = to->InputPin(input.pin_id_);
        return Failure("Types incompatibles : la sortie " +
                           TypeLabel(out_pin->type_, type_name) +
                           " ne peut pas aller dans l'entrée " +
                           TypeLabel(in_pin->type_, type_name) + ".",
                       end);
    } catch (const std::exception &error) {
        return Failure(std::string("Connexion impossible : ") + error.what(),
                       end);
    }
}

bool ConnectionOutcome::Succeeded() const noexcept {
    return connection_.has_value();
}

}  // namespace editor::ui
