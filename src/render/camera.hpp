/**
 * @file camera.hpp
 * @brief Declaration of the Camera class, which handles 2D camera transformations
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 26-09-2026
 */

#pragma once

#include "utils/raylib_wrapper.hpp"
 
namespace render {
 
// Converts between world space (where nodes and links live) and screen space
// (pixels), and owns pan/zoom state. Deliberately raylib-free — it only uses
// utils::WrappedVector2, never <raylib.h> directly — matching the project
// convention that raw raylib types stay inside utils::raylib_wrapper (and,
// for the one thing the wrapper doesn't cover, inside renderer.cpp; see
// BuildRaylibCamera2D there). This is also what makes Camera unit-testable
// without ever calling InitWindow().
//
// WorldToScreen/ScreenToWorld are the single source of truth for the
// mapping. Pan() and ZoomAt() are both implemented in terms of them, so
// there is exactly one formula in the codebase.
class Camera {
   public:
    static constexpr float kMinZoom = 0.25F;
    static constexpr float kMaxZoom = 4.0F;
 
    Camera() = default;
 
    // --- conversion (the one place this math exists) ------------------------
    [[nodiscard]] utils::WrappedVector2 WorldToScreen(
        utils::WrappedVector2 world) const noexcept;
    [[nodiscard]] utils::WrappedVector2 ScreenToWorld(
        utils::WrappedVector2 screen) const noexcept;
 
    // --- input-driven mutation, expressed via the conversion above ----------
 
    // Shifts the view so world content follows the cursor by screen_delta
    // pixels (e.g. a middle-drag delta).
    void Pan(utils::WrappedVector2 screen_delta) noexcept;
 
    // Multiplies the zoom by kZoomStep^wheel_delta (so +1/-1 notches feel
    // exponential rather than linear), clamped to [kMinZoom, kMaxZoom], while
    // keeping the world point currently under screen_anchor fixed on screen.
    void ZoomAt(utils::WrappedVector2 screen_anchor,
               float wheel_delta) noexcept;
 
    // Must be called whenever the window is resized: the viewport center is
    // part of the conversion formula.
    void SetViewport(float width, float height) noexcept;
 
    [[nodiscard]] float Zoom() const noexcept { return zoom_; }
    [[nodiscard]] utils::WrappedVector2 Target() const noexcept {
        return target_;
    }
    [[nodiscard]] utils::WrappedVector2 ViewportCenter() const noexcept {
        return utils::WrappedVector2{viewport_width_ / 2.0F,
                                     viewport_height_ / 2.0F};
    }
 
   private:
    static constexpr float kZoomStep = 1.1F;
 
    utils::WrappedVector2 target_{0.0F, 0.0F};
    float zoom_ = 1.0F;
    float viewport_width_ = 0.0F;
    float viewport_height_ = 0.0F;
};
 
}  // namespace render
