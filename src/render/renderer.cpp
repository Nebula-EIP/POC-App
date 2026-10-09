/**
 * @file renderer.cpp
 * @brief Implementation of the Renderer class, which manages the rendering loop
 * and drawing of the graph
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 06-10-2026
 */

#include "renderer.hpp"

#include <raylib.h>

#include <cmath>
#include <stdexcept>
#include <utility>

namespace render {

namespace {

constexpr utils::WrappedColor kGridColor = utils::kDarkgray;
constexpr Color kBackgroundColor = Color{30, 30, 30, 255};

::Camera2D BuildRaylibCamera2D(const Camera &camera) {
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
    // FLAG_WINDOW_HIGHDPI is only reliable on macOS. On Windows, raylib 5.5
    // does not refresh its DPI scale on resize (maximize, fullscreen), so the
    // mouse position and the drawing drift apart. GLFW already makes the
    // process DPI aware there, so the window simply works in real pixels.
    unsigned int flags = FLAG_WINDOW_RESIZABLE;
#if defined(__APPLE__)
    flags |= FLAG_WINDOW_HIGHDPI;
#endif
    SetConfigFlags(flags);
    InitWindow(width, height, title.c_str());

    if (!utils::IsRaylibReady()) {
        throw std::runtime_error("Raylib window initialization failed");
    }
    window_ready_ = true;
    utils::SetFPS(60);
    SetExitKey(KEY_NULL);
}

Renderer::~Renderer() {
    if (window_ready_ && utils::IsRaylibReady()) {
        utils::CloseRaylib();
    }
}

bool Renderer::ShouldClose() const noexcept {
    return utils::ShouldCloseRaylib();
}

void Renderer::ProcessInput(Camera &camera, bool pointer_captured) noexcept {
    const utils::WrappedVector2 kScreen = ScreenSize();
    camera.SetViewport(kScreen.x_, kScreen.y_);

    if (pointer_captured) {
        return;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        const Vector2 kDelta = GetMouseDelta();
        camera.Pan(utils::WrappedVector2{kDelta.x, kDelta.y});
    }

    const float kWheel = GetMouseWheelMove();
    if (kWheel != 0.0F) {
        camera.ZoomAt(utils::GetCursorPositionWrapped(), kWheel);
    }

    node_canvas_.ProcessInput(camera);
}

void Renderer::RenderFrame(const Camera &camera,
                           const FrameCallbacks &callbacks) {
    utils::BeginFrame();
    ClearBackground(kBackgroundColor);

    BeginMode2D(BuildRaylibCamera2D(camera));
    DrawGrid(camera);
    if (callbacks.draw_links_) {
        callbacks.draw_links_(camera);
    }
    node_canvas_.Draw(camera);
    EndMode2D();

    if (callbacks.draw_ui_) {
        callbacks.draw_ui_();
    }

    utils::EndFrame();
}

void Renderer::Run(Camera &camera, const FrameCallbacks &callbacks) {
    while (!ShouldClose()) {
        ProcessInput(camera);
        RenderFrame(camera, callbacks);
    }
}

float Renderer::DpiScale() const noexcept {
    const Vector2 kScale = GetWindowScaleDPI();
    return kScale.x;
}

utils::WrappedVector2 Renderer::ScreenSize() const noexcept {
    return utils::WrappedVector2{static_cast<float>(GetScreenWidth()),
                                 static_cast<float>(GetScreenHeight())};
}

void Renderer::DrawGrid(const Camera &camera) const {
    const utils::WrappedVector2 kScreen = ScreenSize();
    const utils::WrappedVector2 kTopLeft =
        camera.ScreenToWorld(utils::WrappedVector2{0.0F, 0.0F});
    const utils::WrappedVector2 kBottomRight = camera.ScreenToWorld(kScreen);

    const float kThickness = 1.0F / camera.Zoom();

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