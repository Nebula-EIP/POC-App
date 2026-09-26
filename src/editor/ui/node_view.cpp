/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 26-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 26-09-2026
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

utils::WrappedVector2 Camera::ScreenToWorld(utils::WrappedVector2 point) const
    noexcept {
    return {(point.x_ - offset_.x_) / zoom_, (point.y_ - offset_.y_) / zoom_};
}

utils::WrappedVector2 Camera::WorldToScreen(utils::WrappedVector2 point) const
    noexcept {
    return {point.x_ * zoom_ + offset_.x_, point.y_ * zoom_ + offset_.y_};
}

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
    const std::size_t rows = std::max(inputs_.size(), outputs_.size());
    bounds_.height_ = kHeaderHeight +
                      static_cast<float>(rows) * kPortRowHeight +
                      kHorizontalPadding;

    for (std::size_t index = 0; index < inputs_.size(); ++index) {
        inputs_[index].bounds_ = {
            bounds_.x_ - kPortRadius,
            bounds_.y_ + kHeaderHeight + static_cast<float>(index) *
                              kPortRowHeight + 7.0F,
            2.0F * kPortRadius, 2.0F * kPortRadius};
    }
    for (std::size_t index = 0; index < outputs_.size(); ++index) {
        outputs_[index].bounds_ = {
            bounds_.x_ + bounds_.width_ - kPortRadius,
            bounds_.y_ + kHeaderHeight + static_cast<float>(index) *
                              kPortRowHeight + 7.0F,
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
        const utils::WrappedCircle circle{port.bounds_.x_ + kPortRadius,
                                          port.bounds_.y_ + kPortRadius,
                                          kPortRadius + 2.0F};
        if (utils::CheckCollisionPointCircleWrapped(point, circle)) {
            return {node_id_, port.id_, HitPart::kInputPin};
        }
    }
    for (const auto &port : outputs_) {
        const utils::WrappedCircle circle{port.bounds_.x_ + kPortRadius,
                                          port.bounds_.y_ + kPortRadius,
                                          kPortRadius + 2.0F};
        if (utils::CheckCollisionPointCircleWrapped(point, circle)) {
            return {node_id_, port.id_, HitPart::kOutputPin};
        }
    }
    if (utils::CheckCollisionPointRecWrapped(point, bounds_)) {
        return {node_id_, 0, HitPart::kBody};
    }
    return {};
}

utils::WrappedColor NodeView::TypeColor(core::DataType type) noexcept {
    constexpr utils::WrappedColor palette[] = {utils::kSkyblue, utils::kGreen,
                                                utils::kOrange, utils::kPink,
                                                utils::kPurple, utils::kYellow};
    return palette[type % (sizeof(palette) / sizeof(palette[0]))];
}

void NodeView::Draw(const Camera &camera) const {
    const auto color = TypeColor(node_type_);
    const auto top_left = camera.WorldToScreen({bounds_.x_, bounds_.y_});
    const float width = bounds_.width_ * camera.zoom_;
    const float height = bounds_.height_ * camera.zoom_;
    utils::DrawRectangleWrapped(top_left.x_, top_left.y_, width, height, kBody);
    utils::DrawRectangleWrapped(top_left.x_, top_left.y_, width,
                                kHeaderHeight * camera.zoom_, color);
    utils::DrawRectangleLinesWrapped(
        top_left.x_, top_left.y_, width, height,
        selected_ ? kSelected : kBorder);
    const int scaled_font = std::max(8, static_cast<int>(kFontSize * camera.zoom_));
    utils::DrawTextWrapped(title_.c_str(),
                           top_left.x_ + kHorizontalPadding * camera.zoom_,
                           top_left.y_ + 6.0F * camera.zoom_, scaled_font, kText);
    for (const auto &port : inputs_) {
        const auto port_screen = camera.WorldToScreen(
            {port.bounds_.x_ + kPortRadius, port.bounds_.y_ + kPortRadius});
        utils::DrawCircleWrapped(port_screen.x_, port_screen.y_,
                                 kPortRadius * camera.zoom_,
                                 TypeColor(port.type_));
        const auto text_screen = camera.WorldToScreen(
            {port.bounds_.x_ + 12.0F, port.bounds_.y_ + 1.0F});
        utils::DrawTextWrapped(port.name_.c_str(), text_screen.x_, text_screen.y_,
                               scaled_font, kMutedText);
    }
    for (const auto &port : outputs_) {
        const auto port_screen = camera.WorldToScreen(
            {port.bounds_.x_ + kPortRadius, port.bounds_.y_ + kPortRadius});
        utils::DrawCircleWrapped(port_screen.x_, port_screen.y_,
                                 kPortRadius * camera.zoom_,
                                 TypeColor(port.type_));
        const float width = TextWidth(port.name_);
        const auto text_screen = camera.WorldToScreen(
            {port.bounds_.x_ - width - 8.0F, port.bounds_.y_ + 1.0F});
        utils::DrawTextWrapped(port.name_.c_str(), text_screen.x_, text_screen.y_,
                               scaled_font, kMutedText);
    }
}

}  // namespace editor::ui
