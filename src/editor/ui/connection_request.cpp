/**
 * @file connection_request.cpp
 * @brief Implementation of the canvas connection request
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 24-10-2026
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
        .replaced_ = {},
    };
}

std::string TypeLabel(core::DataType type, const TypeNameResolver &type_name) {
    std::string name = type_name ? type_name(type) : std::string{};
    if (name.empty()) {
        name = "type " + std::to_string(type);
    }
    return "« " + name + " »";
}

const core::Pin *FindPin(const core::Graph &graph,
                         const HitResult &hit) noexcept {
    const core::Node *node = graph.GetNode(hit.node_id_);
    if (node == nullptr) {
        return nullptr;
    }
    return hit.part_ == HitPart::kInputPin ? node->InputPin(hit.pin_id_)
                                           : node->OutputPin(hit.pin_id_);
}

PinEnd ToPinEnd(const core::Graph &graph, const HitResult &hit) noexcept {
    PinEnd pin_end{.node_ = hit.node_id_, .part_ = hit.part_, .type_ = {}};
    if (IsPin(hit)) {
        if (const core::Pin *pin = FindPin(graph, hit); pin != nullptr) {
            pin_end.type_ = pin->type_;
        }
    }
    return pin_end;
}

ConnectionOutcome TypeMismatch(const core::Graph &graph,
                               const HitResult &output, const HitResult &input,
                               const HitResult &end,
                               const TypeNameResolver &type_name) {
    const core::Pin *out_pin = FindPin(graph, output);
    const core::Pin *in_pin = FindPin(graph, input);
    if (out_pin == nullptr || in_pin == nullptr) {
        return Failure("Connexion impossible : types incompatibles.", end);
    }
    return Failure("Types incompatibles : la sortie " +
                       TypeLabel(out_pin->type_, type_name) +
                       " ne peut pas aller dans l'entrée " +
                       TypeLabel(in_pin->type_, type_name) + ".",
                   end);
}

}  // namespace

ConnectionRule CheckConnectionRule(const PinEnd &start,
                                   const PinEnd &end) noexcept {
    const auto kIsPin = [](HitPart part) {
        return part == HitPart::kInputPin || part == HitPart::kOutputPin;
    };
    if (!kIsPin(start.part_) || !kIsPin(end.part_)) {
        return ConnectionRule::kNotAPin;
    }
    if (start.part_ == end.part_) {
        return ConnectionRule::kSameDirection;
    }
    if (start.node_ && end.node_ && *start.node_ == *end.node_) {
        return ConnectionRule::kSameNode;
    }
    if (start.type_ && end.type_ && *start.type_ != *end.type_) {
        return ConnectionRule::kTypeMismatch;
    }
    return ConnectionRule::kAllowed;
}

ConnectionOutcome RequestConnection(core::Graph &graph, const HitResult &start,
                                    const HitResult &end,
                                    const TypeNameResolver &type_name) {
    // Released on the pin it started from: a click, not a connection.
    if (start.node_id_ == end.node_id_ && start.pin_id_ == end.pin_id_ &&
        start.part_ == end.part_) {
        return {};
    }
    const HitResult &output = start.part_ == HitPart::kOutputPin ? start : end;
    const HitResult &input = start.part_ == HitPart::kInputPin ? start : end;

    switch (CheckConnectionRule(ToPinEnd(graph, start), ToPinEnd(graph, end))) {
        case ConnectionRule::kAllowed:
            break;
        case ConnectionRule::kNotAPin:
            return Failure("Relâchez le lien sur une entrée ou une sortie.",
                           end);
        case ConnectionRule::kSameDirection:
            return Failure(start.part_ == HitPart::kInputPin
                               ? "Impossible de relier deux entrées : reliez "
                                 "une sortie à une entrée."
                               : "Impossible de relier deux sorties : reliez "
                                 "une sortie à une entrée.",
                           end);
        case ConnectionRule::kSameNode:
            return Failure("Un node ne peut pas être relié à lui-même.", end);
        case ConnectionRule::kTypeMismatch:
            return TypeMismatch(graph, output, input, end, type_name);
    }

    try {
        ConnectionOutcome outcome;
        try {
            outcome.connection_ = graph.Connect(output.node_id_, output.pin_id_,
                                                input.node_id_, input.pin_id_);
        } catch (const core::InputAlreadyConnectedException &) {
            // The new connection takes priority over the one on the input.
            // Types are checked before, so connecting again cannot fail on
            // them.
            outcome.replaced_ =
                graph.PinConnections(input.node_id_, input.pin_id_, true);
            graph.DisconnectInputPin(input.node_id_, input.pin_id_);
            outcome.connection_ = graph.Connect(output.node_id_, output.pin_id_,
                                                input.node_id_, input.pin_id_);
        }
        return outcome;
    } catch (const core::TypeMismatchException &) {
        return TypeMismatch(graph, output, input, end, type_name);
    } catch (const std::exception &error) {
        return Failure(std::string("Connexion impossible : ") + error.what(),
                       end);
    }
}

bool ConnectionOutcome::Succeeded() const noexcept {
    return connection_.has_value();
}

}  // namespace editor::ui
