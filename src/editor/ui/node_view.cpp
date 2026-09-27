/**
 * @file node_view.cpp
 * @brief Implementation of the NodeView class, which represents a visual node
 * in the editor's UI.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 27-09-2026
 */

#include "node_view.hpp"

#include <algorithm>
#include <cmath>

namespace editor::ui {

namespace {
constexpr float kHeaderHeight = 30.0F;
constexpr float kPortRowHeight = 24.0F;
constexpr float kHorizontalPadding = 12.0F;
constexpr float kPortRadius = 5.0F;
constexpr float kMinWidth = 150.0F;
constexpr float kMaxWidth = 360.0F;
constexpr int kFontSize = 16;
constexpr utils::WrappedColor kBody = {42, 47, 56, 255};
constexpr utils::WrappedColor kBorder = {220, 225, 232, 255};
constexpr utils::WrappedColor kSelected = {255, 214, 72, 255};
constexpr utils::WrappedColor kText = {245, 247, 250, 255};
constexpr utils::WrappedColor kMutedText = {185, 193, 204, 255};

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
    float content_width = TextWidth(title_);
    for (const auto &pin : node.InputPins()) {
        content_width = std::max(content_width, TextWidth(pin.name_));
        inputs_.push_back({pin.id_, pin.name_, pin.type_, {}});
    }
    for (const auto &pin : node.OutputPins()) {
        content_width = std::max(content_width, TextWidth(pin.name_));
        outputs_.push_back({pin.id_, pin.name_, pin.type_, {}});
    }

    bounds_.width_ = std::clamp(content_width + 2.0F * kHorizontalPadding,
                                kMinWidth, kMaxWidth);
    const std::size_t kRows = std::max(inputs_.size(), outputs_.size());
    bounds_.height_ = kHeaderHeight +
                      static_cast<float>(kRows) * kPortRowHeight +
                      kHorizontalPadding;

    for (std::size_t index = 0; index < inputs_.size(); ++index) {
        inputs_[index].bounds_ = {
            bounds_.x_ - kPortRadius,
            bounds_.y_ + kHeaderHeight +
                static_cast<float>(index) * kPortRowHeight + 7.0F,
            2.0F * kPortRadius, 2.0F * kPortRadius};
    }
    for (std::size_t index = 0; index < outputs_.size(); ++index) {
        outputs_[index].bounds_ = {
            bounds_.x_ + bounds_.width_ - kPortRadius,
            bounds_.y_ + kHeaderHeight +
                static_cast<float>(index) * kPortRowHeight + 7.0F,
            2.0F * kPortRadius, 2.0F * kPortRadius};
    }
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
                                           kPortRadius + 2.0F};
        if (utils::CheckCollisionPointCircleWrapped(point, kCircle)) {
            return {node_id_, port.id_, HitPart::kInputPin};
        }
    }
    for (const auto &port : outputs_) {
        const utils::WrappedCircle kCircle{port.bounds_.x_ + kPortRadius,
                                           port.bounds_.y_ + kPortRadius,
                                           kPortRadius + 2.0F};
        if (utils::CheckCollisionPointCircleWrapped(point, kCircle)) {
            return {node_id_, port.id_, HitPart::kOutputPin};
        }
    }
    if (utils::CheckCollisionPointRecWrapped(point, bounds_)) {
        return {node_id_, 0, HitPart::kBody};
    }
    return {};
}

utils::WrappedColor NodeView::TypeColor(core::DataType type) noexcept {
    constexpr utils::WrappedColor kPalette[] = {
        utils::kSkyblue, utils::kGreen,  utils::kOrange,
        utils::kPink,    utils::kPurple, utils::kYellow};
    return kPalette[type % (sizeof(kPalette) / sizeof(kPalette[0]))];
}

void NodeView::Draw(const render::Camera &camera) const {
    const auto kColor = TypeColor(node_type_);
    const auto kTopLeft = camera.WorldToScreen({bounds_.x_, bounds_.y_});
    const float kZoom = camera.Zoom();
    const float kWidth = bounds_.width_ * kZoom;
    const float kHeight = bounds_.height_ * kZoom;
    utils::DrawRectangleWrapped(kTopLeft.x_, kTopLeft.y_, kWidth, kHeight,
                                kBody);
    utils::DrawRectangleWrapped(kTopLeft.x_, kTopLeft.y_, kWidth,
                                kHeaderHeight * kZoom, kColor);
    utils::DrawRectangleLinesWrapped(kTopLeft.x_, kTopLeft.y_, kWidth, kHeight,
                                     selected_ ? kSelected : kBorder);
    const int kScaledFont = std::max(8, static_cast<int>(kFontSize * kZoom));
    utils::DrawTextWrapped(
        title_.c_str(), kTopLeft.x_ + kHorizontalPadding * kZoom,
        kTopLeft.y_ + 6.0F * kZoom, kScaledFont, kText);
    for (const auto &port : inputs_) {
        const auto kPortScreen = camera.WorldToScreen(
            {port.bounds_.x_ + kPortRadius, port.bounds_.y_ + kPortRadius});
        utils::DrawCircleWrapped(kPortScreen.x_, kPortScreen.y_,
                                 kPortRadius * kZoom, TypeColor(port.type_));
        const auto kTextScreen = camera.WorldToScreen(
            {port.bounds_.x_ + 12.0F, port.bounds_.y_ + 1.0F});
        utils::DrawTextWrapped(port.name_.c_str(), kTextScreen.x_,
                               kTextScreen.y_, kScaledFont, kMutedText);
    }
    for (const auto &port : outputs_) {
        const auto kPortScreen = camera.WorldToScreen(
            {port.bounds_.x_ + kPortRadius, port.bounds_.y_ + kPortRadius});
        utils::DrawCircleWrapped(kPortScreen.x_, kPortScreen.y_,
                                 kPortRadius * kZoom, TypeColor(port.type_));
        const float kPortWidth = TextWidth(port.name_);
        const auto kTextScreen = camera.WorldToScreen(
            {port.bounds_.x_ - kPortWidth - 8.0F, port.bounds_.y_ + 1.0F});
        utils::DrawTextWrapped(port.name_.c_str(), kTextScreen.x_,
                               kTextScreen.y_, kScaledFont, kMutedText);
    }
}

}  // namespace editor::ui