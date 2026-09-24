/**
 * @file create_module.cpp
 * @brief Hook used to retreive the C module
 *
 * @author Created by Nolan Papa
 * @date Created on 20-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 20-09-2026
 */

#include "component_adapter.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <type_traits>

namespace {

using core::capa::Button;
using core::capa::Checkbox;
using core::capa::Component;
using core::capa::InfoBubble;
using core::capa::Label;
using core::capa::NumberField;
using core::capa::Select;
using core::capa::Separator;
using core::capa::Slider;
using core::capa::TextField;
using utils::WrappedColor;
using utils::WrappedRectangle;

constexpr int kFontSize = 18;
constexpr float kRowHeight = 30.0F;
constexpr WrappedColor kText = utils::kWhite;
constexpr WrappedColor kPanel = utils::kDarkgray;
constexpr WrappedColor kControl = utils::kGray;
constexpr WrappedColor kDisabled = utils::kLightgray;

bool Hovered(WrappedRectangle rectangle) {
    const auto kCursor = utils::GetCursorPositionWrapped();
    return utils::CheckCollisionPointRecWrapped(kCursor, rectangle);
}

float Clamp(float value, float minimum, float maximum) {
    return std::clamp(value, minimum, maximum);
}

void DrawField(WrappedRectangle rectangle, const std::string &label,
               const std::string &value, bool enabled) {
    utils::DrawTextWrapped(label.c_str(), rectangle.x_, rectangle.y_, kFontSize,
                           enabled ? kText : kDisabled);
    utils::DrawRectangleWrapped(rectangle.x_ + 100.0F, rectangle.y_,
                                rectangle.width_ - 100.0F, rectangle.height_,
                                enabled ? kControl : kPanel);
    utils::DrawTextWrapped(value.c_str(), rectangle.x_ + 108.0F,
                           rectangle.y_ + 5.0F, kFontSize, kText);
}

}  // namespace

namespace editor::ui {

void ComponentAdapter::Draw(core::NodeId node_id,
                            const core::capa::ComponentList &components,
                            WrappedRectangle bounds) {
    NodeState &state = states_[node_id];
    float y = bounds.y_;
    for (const Component &component : components) {
        const WrappedRectangle kRow{bounds.x_, y, bounds.width_, kRowHeight};
        std::visit(
            [&](const auto &value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, Label>) {
                    utils::DrawTextWrapped(value.text_.c_str(), kRow.x_,
                                           kRow.y_, kFontSize, kText);
                } else if constexpr (std::is_same_v<T, InfoBubble>) {
                    utils::DrawTextWrapped(value.text_.c_str(), kRow.x_,
                                           kRow.y_, kFontSize, utils::kYellow);
                } else if constexpr (std::is_same_v<T, Separator>) {
                    utils::DrawRectangleWrapped(kRow.x_, kRow.y_ + 14.0F,
                                                kRow.width_, 1.0F, kControl);
                } else if constexpr (std::is_same_v<T, Button>) {
                    DrawField(kRow, value.label_, "", value.enabled_);
                    if (value.enabled_ && Hovered(kRow) &&
                        utils::IsLeftClicked() && value.on_click_) {
                        value.on_click_();
                    }
                } else if constexpr (std::is_same_v<T, TextField>) {
                    auto &buffer = state.text_[value.id_];
                    if (buffer.empty() && !value.value_.empty()) {
                        buffer = value.value_;
                    }
                    DrawField(kRow, value.label_, buffer,
                              value.enabled_ && !value.read_only_);
                    if (value.enabled_ && !value.read_only_ && Hovered(kRow) &&
                        utils::IsLeftClicked()) {
                        state.focused_component_ = value.id_;
                    }
                    if (state.focused_component_ == value.id_ &&
                        value.enabled_ && !value.read_only_) {
                        for (int character = utils::GetCharPressedWrapped();
                             character > 0;
                             character = utils::GetCharPressedWrapped()) {
                            buffer.push_back(static_cast<char>(character));
                        }
                        if (utils::IsKeyPressedWrapped(
                                utils::WrappedKey::kBackspace) &&
                            !buffer.empty()) {
                            buffer.pop_back();
                        }
                        if (value.on_change_) value.on_change_(buffer);
                    }
                } else if constexpr (std::is_same_v<T, NumberField>) {
                    char text[64];
                    std::snprintf(text, sizeof(text),
                                  value.is_integer_ ? "%.0f" : "%.3f",
                                  value.value_);
                    DrawField(kRow, value.label_, text, value.enabled_);
                    if (value.enabled_ && Hovered(kRow) &&
                        utils::IsLeftClicked() && value.on_change_) {
                        const double kNext = std::clamp(
                            value.value_ + value.step_, value.min_, value.max_);
                        value.on_change_(value.is_integer_ ? std::round(kNext)
                                                           : kNext);
                    }
                } else if constexpr (std::is_same_v<T, Slider>) {
                    DrawField(kRow, value.label_, std::to_string(value.value_),
                              value.enabled_);
                    if (value.enabled_ && value.max_ > value.min_ &&
                        Hovered(kRow) && utils::IsLeftDown() &&
                        value.on_change_) {
                        const auto kCursor = utils::GetCursorPositionWrapped();
                        const double kRatio = Clamp(
                            (kCursor.x_ - kRow.x_) / kRow.width_, 0.0F, 1.0F);
                        double next =
                            value.min_ + kRatio * (value.max_ - value.min_);
                        if (value.step_ > 0.0) {
                            next = value.min_ + std::round((next - value.min_) /
                                                           value.step_) *
                                                    value.step_;
                        }
                        value.on_change_(
                            std::clamp(next, value.min_, value.max_));
                    }
                } else if constexpr (std::is_same_v<T, Checkbox>) {
                    DrawField(kRow, value.label_,
                              value.value_ ? "true" : "false", value.enabled_);
                    if (value.enabled_ && Hovered(kRow) &&
                        utils::IsLeftClicked() && value.on_change_) {
                        value.on_change_(!value.value_);
                    }
                } else if constexpr (std::is_same_v<T, Select>) {
                    const std::string kSelected =
                        value.options_.empty() ||
                                value.selected_index_ >= value.options_.size()
                            ? "<invalid>"
                            : value.options_[value.selected_index_];
                    DrawField(kRow, value.label_, kSelected, value.enabled_);
                    if (value.enabled_ && !value.options_.empty() &&
                        Hovered(kRow) && utils::IsLeftClicked() &&
                        value.on_change_) {
                        value.on_change_((value.selected_index_ + 1) %
                                         value.options_.size());
                    }
                } else {
                    utils::DrawTextWrapped("Unsupported component", kRow.x_,
                                           kRow.y_, kFontSize, utils::kRed);
                }
            },
            component);
        y += kRowHeight;
    }
}

void ComponentAdapter::ClearNode(core::NodeId node_id) noexcept {
    states_.erase(node_id);
}

void ComponentAdapter::Clear() noexcept { states_.clear(); }

bool ComponentAdapter::HasState(core::NodeId node_id) const noexcept {
    return states_.contains(node_id);
}

}  // namespace editor::ui
