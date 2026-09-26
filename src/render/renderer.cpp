/**
 * @file renderer.cpp
 * @brief Implementation of the Renderer class, which manages the rendering loop and drawing of the graph
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 26-09-2026
 */

#include "renderer.hpp"

#include <cmath>
#include <raylib.h>
#include <stdexcept>
#include <utility>

namespace render {

namespace {

constexpr utils::WrappedColor kGridColor = utils::kDarkgray;
// Not in utils::WrappedColor's named palette and utils::ClearScreen() is
// hardcoded to RAYWHITE, so the background clear stays a raw raylib call.
constexpr Color kBackgroundColor = Color{30, 30, 30, 255};

// The one place a raylib type (Camera2D) gets built, entirely internal to
// this file — Camera itself never sees it. Built from the same
// Zoom()/Target()/ViewportCenter() that WorldToScreen/ScreenToWorld use, so
// the GPU transform (BeginMode2D) and manual hit-testing can never disagree.
::Camera2D BuildRaylibCamera2D(const Camera& camera) {
    const utils::WrappedVector2 kCenter = camera.ViewportCenter();
    const utils::WrappedVector2 kTarget = camera.Target();
    return ::Camera2D{
        .offset = Vector2{kCenter.x_, kCenter.y_},
        .target = Vector2{kTarget.x_, kTarget.y_},
        .rotation = 0.0F,
        .zoom = camera.Zoom(),
    };
}

}  // namespace

Renderer::Renderer(int width, int height, std::string title) {
    // utils::InitRaylib() doesn't expose FLAG_WINDOW_HIGHDPI, which the
    // HiDPI acceptance criterion needs, so window creation is one of the few
    // places this module talks to raylib directly instead of through the
    // wrapper.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(width, height, title.c_str());

    if (!utils::IsRaylibReady()) {
        throw std::runtime_error("Raylib window initialization failed");
    }
    window_ready_ = true;
    utils::SetFPS(60);
}

Renderer::~Renderer() {
    if (window_ready_ && utils::IsRaylibReady()) {
        utils::CloseRaylib();
    }
}

bool Renderer::ShouldClose() const noexcept {
    return utils::ShouldCloseRaylib();
}

void Renderer::ProcessInput(Camera& camera) noexcept {
    const utils::WrappedVector2 kScreen = ScreenSize();
    camera.SetViewport(kScreen.x_, kScreen.y_);

    // No wrapper coverage for the middle mouse button or a raw delta read,
    // so these stay direct raylib calls, local to this function.
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        const Vector2 kDelta = GetMouseDelta();
        camera.Pan(utils::WrappedVector2{kDelta.x, kDelta.y});
    }

    const float kWheel = GetMouseWheelMove();
    if (kWheel != 0.0F) {
        camera.ZoomAt(utils::GetCursorPositionWrapped(), kWheel);
    }
}

void Renderer::RenderFrame(const Camera& camera,
                           const FrameCallbacks& callbacks) {
    utils::BeginFrame();
    // Custom dark background — utils::ClearScreen() is fixed to RAYWHITE, so
    // this stays a direct call.
    ClearBackground(kBackgroundColor);

    // World-space layers share the camera's GPU transform (see
    // BuildRaylibCamera2D above for why that stays consistent with
    // WorldToScreen/ScreenToWorld).
    BeginMode2D(BuildRaylibCamera2D(camera));
    DrawGrid(camera);
    if (callbacks.draw_links) {
        callbacks.draw_links(camera);
    }
    if (callbacks.draw_nodes) {
        callbacks.draw_nodes(camera);
    }
    EndMode2D();

    // UI is screen-space and always drawn last, so it is never occluded by
    // graph content and never affected by pan/zoom.
    if (callbacks.draw_ui) {
        callbacks.draw_ui();
    }

    utils::EndFrame();
}

void Renderer::Run(Camera& camera, const FrameCallbacks& callbacks) {
    while (!ShouldClose()) {
        ProcessInput(camera);
        RenderFrame(camera, callbacks);
    }
}

float Renderer::DpiScale() const noexcept {
    const Vector2 kScale = GetWindowScaleDPI();  // not covered by the wrapper
    // x and y are equal on every platform raylib supports; expose one number.
    return kScale.x;
}

utils::WrappedVector2 Renderer::ScreenSize() const noexcept {
    return utils::WrappedVector2{static_cast<float>(GetScreenWidth()),
                                 static_cast<float>(GetScreenHeight())};
}

void Renderer::DrawGrid(const Camera& camera) const {
    const utils::WrappedVector2 kScreen = ScreenSize();
    const utils::WrappedVector2 kTopLeft =
        camera.ScreenToWorld(utils::WrappedVector2{0.0F, 0.0F});
    const utils::WrappedVector2 kBottomRight = camera.ScreenToWorld(kScreen);

    const float kThickness = 1.0F / camera.Zoom();  // stays ~1px on screen

    const float kFirstVerticalLine =
        std::floor(kTopLeft.x_ / kGridSpacing) * kGridSpacing;
    for (float x = kFirstVerticalLine; x <= kBottomRight.x_;
        x += kGridSpacing) {
        utils::DrawLineWrapped(utils::WrappedVector2{x, kTopLeft.y_},
                               utils::WrappedVector2{x, kBottomRight.y_},
                               kThickness, kGridColor);
    }

    const float kFirstHorizontalLine =
        std::floor(kTopLeft.y_ / kGridSpacing) * kGridSpacing;
    for (float y = kFirstHorizontalLine; y <= kBottomRight.y_;
        y += kGridSpacing) {
        utils::DrawLineWrapped(utils::WrappedVector2{kTopLeft.x_, y},
                               utils::WrappedVector2{kBottomRight.x_, y},
                               kThickness, kGridColor);
    }
}

}  // namespace render