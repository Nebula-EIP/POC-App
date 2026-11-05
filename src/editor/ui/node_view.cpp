/**
 * @file node_view.cpp
 * @brief Implementation of the NodeView class, which represents a visual node
 * in the editor's UI.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 05-11-2026
 */

#include "node_view.hpp"

#include <algorithm>
#include <cmath>

#include "utils/palette.hpp"

namespace editor::ui {

namespace {
constexpr float kHeaderHeight = 30.0F;
constexpr float kPortRowHeight = 28.0F;
constexpr float kHorizontalPadding = 12.0F;
constexpr float kPortRadius = 5.0F;
constexpr float kMinWidth = 150.0F;
constexpr float kMaxWidth = 360.0F;
// Space between the pins and the node's fields.
constexpr float kContentGap = 4.0F;
constexpr int kFontSize = 16;
constexpr float kSpacing = 2.0F;

// Card.
constexpr float kCornerRadius = 6.0F;
constexpr utils::WrappedVector2 kShadowOffset = {0.0F, 2.0F};
constexpr float kShadowBlur = 8.0F;
constexpr float kSelectedOutline = 1.5F;
// Accent line at the top of the header, touching the top of the card:
// thickness, and margin on each side as a share of the width.
constexpr float kAccentThickness = 2.0F;
constexpr float kAccentInset = 0.12F;

// Pins: half discs (data) or triangles (execution flow) on the card's edge,
// centered in their row.
constexpr float kPortTop = kPortRowHeight / 2.0F - kPortRadius;
constexpr float kPinRadius = 8.0F;
constexpr float kPinHitRadius = kPinRadius + 1.0F;
constexpr float kPinRingWidth = 1.5F;
constexpr float kFlowPinWidth = 15.0F;
constexpr float kFlowPinHeight = 17.0F;
// The triangles sit a bit to the right of the edge, pointing right.
constexpr float kFlowPinShift = 3.0F;
constexpr float kFlowPinOutline = 1.5F;
constexpr float kFlowPinLabelGap = 6.0F;

// Angles (degrees, 0 = right, 90 = down) of the half of a pin outside the
// card.
constexpr float kLeftHalfStart = 90.0F;
constexpr float kLeftHalfEnd = 270.0F;
constexpr float kRightHalfStart = -90.0F;
constexpr float kRightHalfEnd = 90.0F;

/**
 * @brief Draws the outline of a triangle with lines of a given width.
 *
 */
void DrawTriangleOutline(utils::WrappedVector2 first,
                         utils::WrappedVector2 second,
                         utils::WrappedVector2 third, float thick,
                         utils::WrappedColor color) {
    utils::DrawLineWrapped(first, second, thick, color);
    utils::DrawLineWrapped(second, third, thick, color);
    utils::DrawLineWrapped(third, first, thick, color);
}

}  // namespace

NodeView::NodeView(const core::Node &node, std::string title,
                   utils::WrappedVector2 position)
    : node_id_(node.Id()), node_type_(node.Type()), title_(std::move(title)) {
    bounds_.x_ = position.x_;
    bounds_.y_ = position.y_;
    RebuildLayout(node);
}

float NodeView::TextWidth(const std::string &text) noexcept {
    return static_cast<float>(text.size()) * 8.0F;
}

void NodeView::RebuildLayout(const core::Node &node) {
    inputs_.clear();
    outputs_.clear();
    for (const auto &pin : node.InputPins()) {
        inputs_.push_back({pin.id_, pin.name_, pin.type_, {}});
    }
    for (const auto &pin : node.OutputPins()) {
        outputs_.push_back({pin.id_, pin.name_, pin.type_, {}});
    }
    Layout();
}

void NodeView::Layout() {
    float content_width = TextWidth(title_);
    for (const auto &port : inputs_) {
        content_width = std::max(content_width, TextWidth(port.name_));
    }
    for (const auto &port : outputs_) {
        content_width = std::max(content_width, TextWidth(port.name_));
    }
    content_width = std::max(content_width, content_size_.x_);

    bounds_.width_ = std::clamp(content_width + 2.0F * kHorizontalPadding,
                                kMinWidth, kMaxWidth);
    const std::size_t kRows = std::max(inputs_.size(), outputs_.size());
    bounds_.height_ = kHeaderHeight +
                      static_cast<float>(kRows) * kPortRowHeight +
                      kHorizontalPadding;
    if (content_size_.y_ > 0.0F) {
        bounds_.height_ += content_size_.y_ + kContentGap;
    }

    for (std::size_t index = 0; index < inputs_.size(); ++index) {
        inputs_[index].bounds_ = {
            bounds_.x_ - kPortRadius,
            bounds_.y_ + kHeaderHeight +
                static_cast<float>(index) * kPortRowHeight + kPortTop,
            2.0F * kPortRadius, 2.0F * kPortRadius};
    }
    for (std::size_t index = 0; index < outputs_.size(); ++index) {
        outputs_[index].bounds_ = {
            bounds_.x_ + bounds_.width_ - kPortRadius,
            bounds_.y_ + kHeaderHeight +
                static_cast<float>(index) * kPortRowHeight + kPortTop,
            2.0F * kPortRadius, 2.0F * kPortRadius};
    }
}

void NodeView::Refresh(const core::Node &node) { RebuildLayout(node); }

void NodeView::SetContentSize(utils::WrappedVector2 size) {
    if (size.x_ == content_size_.x_ && size.y_ == content_size_.y_) {
        return;
    }
    content_size_ = size;
    Layout();
}

utils::WrappedRectangle NodeView::ContentBounds() const noexcept {
    const std::size_t kRows = std::max(inputs_.size(), outputs_.size());
    return {bounds_.x_ + kHorizontalPadding,
            bounds_.y_ + kHeaderHeight +
                static_cast<float>(kRows) * kPortRowHeight + kContentGap,
            bounds_.width_ - 2.0F * kHorizontalPadding, content_size_.y_};
}

void NodeView::MoveBy(utils::WrappedVector2 delta) noexcept {
    bounds_.x_ += delta.x_;
    bounds_.y_ += delta.y_;
    for (auto &port : inputs_) {
        port.bounds_.x_ += delta.x_;
        port.bounds_.y_ += delta.y_;
    }
    for (auto &port : outputs_) {
        port.bounds_.x_ += delta.x_;
        port.bounds_.y_ += delta.y_;
    }
}

HitResult NodeView::HitTest(utils::WrappedVector2 point) const noexcept {
    for (const auto &port : inputs_) {
        const utils::WrappedCircle kCircle{port.bounds_.x_ + kPortRadius,
                                           port.bounds_.y_ + kPortRadius,
                                           kPinHitRadius};
        if (utils::CheckCollisionPointCircleWrapped(point, kCircle)) {
            return {node_id_, port.id_, HitPart::kInputPin};
        }
    }
    for (const auto &port : outputs_) {
        const utils::WrappedCircle kCircle{port.bounds_.x_ + kPortRadius,
                                           port.bounds_.y_ + kPortRadius,
                                           kPinHitRadius};
        if (utils::CheckCollisionPointCircleWrapped(point, kCircle)) {
            return {node_id_, port.id_, HitPart::kOutputPin};
        }
    }
    if (utils::CheckCollisionPointRecWrapped(point, bounds_)) {
        return {node_id_, 0, HitPart::kBody};
    }
    return {};
}

void NodeView::Draw(const render::Camera &camera,
                    const NodePaint &paint) const {
    (void)camera;  // Handled by Raylib's BeginMode2D

    // Card: the header color everywhere, then the body below the header
    // (square on top, rounded at the bottom).
    utils::DrawShadowWrapped(bounds_, kCornerRadius, kShadowOffset, kShadowBlur,
                             palette::node::kShadow);
    utils::DrawRectangleRoundedWrapped(bounds_, kCornerRadius,
                                       palette::node::kHeader);
    const float kBodyHeight = bounds_.height_ - kHeaderHeight;
    if (kBodyHeight > 0.0F) {
        utils::DrawRectangleRoundedWrapped(
            {bounds_.x_, bounds_.y_ + kHeaderHeight, bounds_.width_,
             kBodyHeight},
            kCornerRadius, palette::node::kBody);
        utils::DrawRectangleRecWrapped(
            {bounds_.x_, bounds_.y_ + kHeaderHeight, bounds_.width_,
             std::min(kCornerRadius, kBodyHeight)},
            palette::node::kBody);
    }
    if (selected_) {
        utils::DrawRectangleRoundedLinesWrapped(
            bounds_, kCornerRadius, kSelectedOutline,
            palette::node::kSelectedOutline);
    }

    // Accent line at the top of the card, in the color of the node's kind.
    const utils::WrappedColor kAccent =
        paint.accent_ ? paint.accent_(node_type_) : palette::accent::kNone;
    if (kAccent.a_ > 0) {
        const float kInset = bounds_.width_ * kAccentInset;
        utils::DrawRectangleRoundedWrapped(
            {bounds_.x_ + kInset, bounds_.y_, bounds_.width_ - 2.0F * kInset,
             kAccentThickness},
            kAccentThickness / 2.0F, kAccent);
    }

    utils::DrawTextExWrapped(
        title_.c_str(), bounds_.x_ + kHorizontalPadding,
        bounds_.y_ + (kHeaderHeight - static_cast<float>(kFontSize)) / 2.0F,
        kFontSize, kSpacing, palette::node::kTitle);

    for (const auto &port : inputs_) {
        DrawPort(port, true, paint);
    }
    for (const auto &port : outputs_) {
        DrawPort(port, false, paint);
    }
}

void NodeView::DrawPort(const PortView &port, bool is_input,
                        const NodePaint &paint) const {
    const utils::WrappedVector2 kCenter{port.bounds_.x_ + kPortRadius,
                                        port.bounds_.y_ + kPortRadius};
    const std::string kTypeName =
        paint.type_name_ ? paint.type_name_(port.type_) : std::string{};
    const bool kLinked =
        !paint.pin_linked_ || paint.pin_linked_(node_id_, port.id_, is_input);
    const bool kError =
        port.id_ == error_pin_id_ && error_pin_is_input_ == is_input;
    const utils::WrappedColor kColor =
        kError ? palette::node::kErrorPin : palette::TypeColor(kTypeName);

    if (kTypeName == palette::type::kFlowTypeName) {
        // Execution flow: a triangle pointing right, across the edge.
        const float kX = kCenter.x_ + kFlowPinShift;
        const utils::WrappedVector2 kTop{kX - kFlowPinWidth / 2.0F,
                                         kCenter.y_ - kFlowPinHeight / 2.0F};
        const utils::WrappedVector2 kBottom{kTop.x_,
                                            kCenter.y_ + kFlowPinHeight / 2.0F};
        const utils::WrappedVector2 kTip{kX + kFlowPinWidth / 2.0F, kCenter.y_};
        if (kLinked) {
            utils::DrawTriangleWrapped(kTop, kBottom, kTip, kColor);
        } else {
            DrawTriangleOutline(kTop, kBottom, kTip, kFlowPinOutline, kColor);
        }
    } else {
        // Data: the half of a disc outside the card, a ring when unlinked.
        const float kStart = is_input ? kLeftHalfStart : kRightHalfStart;
        const float kEnd = is_input ? kLeftHalfEnd : kRightHalfEnd;
        if (kLinked) {
            utils::DrawCircleSectorWrapped(kCenter, kPinRadius, kStart, kEnd,
                                           kColor);
        } else {
            utils::DrawRingWrapped(kCenter, kPinRadius - kPinRingWidth,
                                   kPinRadius, kStart, kEnd, kColor);
        }
    }

    const float kTextY = kCenter.y_ - static_cast<float>(kFontSize) / 2.0F;
    if (is_input) {
        // Past the tip of a flow triangle, which reaches into the card.
        const float kTextX = kTypeName == palette::type::kFlowTypeName
                                 ? kCenter.x_ + kFlowPinShift +
                                       kFlowPinWidth / 2.0F + kFlowPinLabelGap
                                 : kCenter.x_ + kHorizontalPadding;
        utils::DrawTextExWrapped(port.name_.c_str(), kTextX, kTextY, kFontSize,
                                 kSpacing, palette::node::kPinLabel);
        return;
    }
    // Measured with the real font, so that the names end on the same line.
    const float kTextWidth =
        utils::MeasureTextExWrapped(port.name_.c_str(), kFontSize, kSpacing);
    utils::DrawTextExWrapped(
        port.name_.c_str(), kCenter.x_ - kHorizontalPadding - kTextWidth,
        kTextY, kFontSize, kSpacing, palette::node::kPinLabel);
}

std::optional<utils::WrappedVector2> NodeView::GetPinPosition(
    core::PinId pin_id, bool is_input) const noexcept {
    if (is_input) {
        for (const auto &port : inputs_) {
            if (port.id_ == pin_id) {
                return utils::WrappedVector2{port.bounds_.x_ + kPortRadius,
                                             port.bounds_.y_ + kPortRadius};
            }
        }
    } else {
        for (const auto &port : outputs_) {
            if (port.id_ == pin_id) {
                return utils::WrappedVector2{port.bounds_.x_ + kPortRadius,
                                             port.bounds_.y_ + kPortRadius};
            }
        }
    }
    return std::nullopt;
}

void NodeView::SetErrorPin(core::PinId pin, bool is_input) noexcept {
    error_pin_id_ = pin;
    error_pin_is_input_ = is_input;
}

}  // namespace editor::ui
