/**
 * @file renderer.cpp
 * @brief Implementation of the Renderer class, which manages the rendering loop
 * and drawing of the graph
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 09-10-2026
 */

#include "renderer.hpp"

#include <raylib.h>

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "grid.hpp"

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

struct GridBounds {
    utils::WrappedVector2 top_left_;
    utils::WrappedVector2 bottom_right_;
};

/**
 * @brief Draws one grid level over the visible world area.
 *
 * @param skip_every Lines whose index is a multiple of this are skipped
 * (they belong to the coarse level, drawn separately); 0 skips none.
 */
void DrawGridLevel(const GridBounds &bounds, float spacing, int skip_every,
                   float thickness, utils::WrappedColor color) {
    const auto kFirstColumn =
        static_cast<std::int64_t>(std::floor(bounds.top_left_.x_ / spacing));
    const auto kLastColumn =
        static_cast<std::int64_t>(std::ceil(bounds.bottom_right_.x_ / spacing));
    for (std::int64_t i = kFirstColumn; i <= kLastColumn; ++i) {
        if (skip_every > 0 && i % skip_every == 0) {
            continue;
        }
        const float kX = static_cast<float>(i) * spacing;
        utils::DrawLineWrapped(
            utils::WrappedVector2{kX, bounds.top_left_.y_},
            utils::WrappedVector2{kX, bounds.bottom_right_.y_}, thickness,
            color);
    }

    const auto kFirstRow =
        static_cast<std::int64_t>(std::floor(bounds.top_left_.y_ / spacing));
    const auto kLastRow =
        static_cast<std::int64_t>(std::ceil(bounds.bottom_right_.y_ / spacing));
    for (std::int64_t i = kFirstRow; i <= kLastRow; ++i) {
        if (skip_every > 0 && i % skip_every == 0) {
            continue;
        }
        const float kY = static_cast<float>(i) * spacing;
        utils::DrawLineWrapped(
            utils::WrappedVector2{bounds.top_left_.x_, kY},
            utils::WrappedVector2{bounds.bottom_right_.x_, kY}, thickness,
            color);
    }
}

}  // namespace

Renderer::Renderer(int width, int height, std::string title) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
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
    const GridBounds kBounds{
        .top_left_ = camera.ScreenToWorld(utils::WrappedVector2{0.0F, 0.0F}),
        .bottom_right_ = camera.ScreenToWorld(kScreen),
    };
    const float kThickness = 1.0F / camera.Zoom();
    const GridLevels kLevels = ComputeGridLevels(camera.Zoom());

    utils::WrappedColor fine_color = kGridColor;
    fine_color.a_ = static_cast<unsigned char>(
        std::lround(kLevels.fine_alpha_ * static_cast<float>(kGridColor.a_)));

    if (fine_color.a_ > 0) {
        DrawGridLevel(kBounds, kLevels.fine_spacing_, kGridSubdivisions,
                      kThickness, fine_color);
    }
    DrawGridLevel(kBounds, kLevels.coarse_spacing_, 0, kThickness, kGridColor);
}

}  // namespace render