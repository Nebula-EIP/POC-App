/**
 * @file camera.hpp
 * @brief Declaration of the Camera class, which handles 2D camera
 * transformations
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 29-09-2026
 */

#pragma once

#include "utils/raylib_wrapper.hpp"

namespace render {

/**
 * @brief A 2D camera that handles panning and zooming, and provides conversion
 * between world and screen coordinates.
 */
class Camera {
   public:
    static constexpr float kMinZoom = 0.25F;
    static constexpr float kMaxZoom = 4.0F;

    Camera() = default;

    [[nodiscard]] utils::WrappedVector2 WorldToScreen(
        utils::WrappedVector2 world) const noexcept;
    [[nodiscard]] utils::WrappedVector2 ScreenToWorld(
        utils::WrappedVector2 screen) const noexcept;

    /**
     * @brief Pans the camera by a given screen delta,
     *        adjusting the target position accordingly.
     *
     * @param screen_delta The amount to pan the camera in screen coordinates.
     *
     * @return void
     */
    void Pan(utils::WrappedVector2 screen_delta) noexcept;

    /**
     * @brief Zooms the camera at a given screen anchor point,
     *        adjusting the zoom level and target position accordingly.
     *
     * @param screen_anchor The anchor point in screen coordinates
     *                      where the zoom is centered.
     * @param wheel_delta The amount to zoom, typically derived from
     *                    mouse wheel input.
     *
     * @return void
     */
    void ZoomAt(utils::WrappedVector2 screen_anchor,
                float wheel_delta) noexcept;

    /**
     * @brief Sets an absolute zoom level, keeping the world point under
     *        screen_anchor in place.
     *
     * @param zoom The requested zoom level, clamped to [kMinZoom, kMaxZoom].
     * @param screen_anchor The screen point that must not move.
     *
     * @return void
     */
    void SetZoom(float zoom, utils::WrappedVector2 screen_anchor) noexcept;

    /**
     * @brief Moves the camera so that a world point is displayed at a given
     *        screen point. The zoom level is left unchanged.
     *
     * @param world The world point to bring into view.
     * @param screen_anchor Where that point must appear on screen.
     *
     * @return void
     */
    void CenterOn(utils::WrappedVector2 world,
                  utils::WrappedVector2 screen_anchor) noexcept;

    /**
     * @brief Sets the viewport dimensions for the camera
     *
     * @param width The width of the viewport in pixels.
     * @param height The height of the viewport in pixels.
     *
     * @return void
     */
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
