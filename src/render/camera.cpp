/**
 * @file camera.cpp
 * @brief Implementation of the Camera class, which handles 2D camera transformations
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 26-09-2026
 */

#include "camera.hpp"

#include <algorithm>
#include <cmath>

namespace render {

utils::WrappedVector2 Camera::WorldToScreen(
    utils::WrappedVector2 world) const noexcept {
    const utils::WrappedVector2 kCenter = ViewportCenter();
    return utils::WrappedVector2{
        (world.x_ - target_.x_) * zoom_ + kCenter.x_,
        (world.y_ - target_.y_) * zoom_ + kCenter.y_};
}

utils::WrappedVector2 Camera::ScreenToWorld(
    utils::WrappedVector2 screen) const noexcept {
    const utils::WrappedVector2 kCenter = ViewportCenter();
    return utils::WrappedVector2{
        (screen.x_ - kCenter.x_) / zoom_ + target_.x_,
        (screen.y_ - kCenter.y_) / zoom_ + target_.y_};
}

void Camera::Pan(utils::WrappedVector2 screen_delta) noexcept {
    // Content must follow the cursor: shifting the target by -delta/zoom
    // moves WorldToScreen's output by +delta. See WorldToScreen above for
    // the derivation this depends on.
    target_.x_ -= screen_delta.x_ / zoom_;
    target_.y_ -= screen_delta.y_ / zoom_;
}

void Camera::ZoomAt(utils::WrappedVector2 screen_anchor,
                    float wheel_delta) noexcept {
    if (wheel_delta == 0.0F) {
        return;
    }

    // Where does the anchor point sit in world space, at the OLD zoom/target?
    const utils::WrappedVector2 kWorldUnderAnchor = ScreenToWorld(screen_anchor);

    const float kRawZoom = zoom_ * std::pow(kZoomStep, wheel_delta);
    zoom_ = std::clamp(kRawZoom, kMinZoom, kMaxZoom);

    // Solve target so that WorldToScreen(kWorldUnderAnchor) == screen_anchor
    // again, at the NEW zoom — i.e. invert WorldToScreen for target_.
    const utils::WrappedVector2 kCenter = ViewportCenter();
    target_.x_ = kWorldUnderAnchor.x_ - (screen_anchor.x_ - kCenter.x_) / zoom_;
    target_.y_ = kWorldUnderAnchor.y_ - (screen_anchor.y_ - kCenter.y_) / zoom_;
}

void Camera::SetViewport(float width, float height) noexcept {
    viewport_width_ = width;
    viewport_height_ = height;
}

}  // namespace render