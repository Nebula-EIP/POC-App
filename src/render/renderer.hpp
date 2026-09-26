/**
 * @file renderer.hpp
 * @brief Declaration of the Renderer class, which manages the rendering loop and drawing of the graph
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 26-09-2026
 */

#pragma once

#include <functional>
#include <string>
 
#include "camera.hpp"
#include "raylib_wrapper.hpp"
 
namespace render {
 
// Draw callbacks for the layers that vary per frame. Grid is drawn by
// Renderer itself (it only needs the camera); links/nodes/UI are owned by
// whoever bridges core::Graph to this module (the editor's graphical
// adapter), so Renderer takes them as callbacks and stays ignorant of core.
//
// draw_links and draw_nodes run inside the camera's 2D transform (world
// space); draw_ui runs in plain screen space, after it, so UI always draws
// on top and is never affected by pan/zoom.
struct FrameCallbacks {
    std::function<void(const Camera&)> draw_links;
    std::function<void(const Camera&)> draw_nodes;
    std::function<void()> draw_ui;
};
 
// Owns the window and the main loop. The render module has no dependency on
// core: it knows about nodes and links only as opaque callbacks.
class Renderer {
   public:
    // Opens the window. Throws std::runtime_error if raylib fails to
    // initialize it.
    Renderer(int width, int height, std::string title);
 
    ~Renderer();
 
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;
 
    [[nodiscard]] bool ShouldClose() const noexcept;
 
    // Reads mouse/window events for this frame and applies them to camera:
    // middle-click drag pans, wheel zooms centered on the cursor, and any
    // window resize updates the viewport. Call once per frame, before
    // RenderFrame.
    void ProcessInput(Camera& camera) noexcept;
 
    // Draws one frame: clears, grid, then callbacks in the fixed order
    // grid -> links -> nodes -> UI.
    void RenderFrame(const Camera& camera, const FrameCallbacks& callbacks);
 
    // Convenience loop: ProcessInput + RenderFrame until the window is asked
    // to close.
    void Run(Camera& camera, const FrameCallbacks& callbacks);
 
    // DPI scale factor (1.0 on most Linux setups, >1.0 on HiDPI Windows
    // displays). UI layers that draw fixed-size text or icons should
    // multiply their sizes by this to stay legible.
    [[nodiscard]] float DpiScale() const noexcept;
 
    // Current framebuffer size, for callers (e.g. Application) that need to
    // feed Camera::SetViewport without touching raylib themselves.
    [[nodiscard]] utils::WrappedVector2 ScreenSize() const noexcept;
 
   private:
    void DrawGrid(const Camera& camera) const;
 
    bool window_ready_ = false;
    static constexpr float kGridSpacing = 50.0F;
};
 
}  // namespace render
