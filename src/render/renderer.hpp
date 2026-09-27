/**
 * @file renderer.hpp
 * @brief Declaration of the Renderer class, which manages the rendering loop
 * and drawing of the graph
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

/**
 * @brief Struct to hold callbacks for rendering different parts of the frame.
 *
 */
struct FrameCallbacks {
    std::function<void(const Camera &)> draw_links_;
    std::function<void(const Camera &)> draw_nodes_;
    std::function<void()> draw_ui_;
};

/**
 * @brief A class that manages the rendering loop and drawing of the graph.
 *
 */
class Renderer {
   public:
    Renderer(int width, int height, std::string title);

    ~Renderer();

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    Renderer &operator=(Renderer &&) = delete;

    [[nodiscard]] bool ShouldClose() const noexcept;

    /**
     * @brief Processes user input events and updates the camera accordingly.
     *
     * @param camera The camera to be updated based on user input.
     *
     * @return void
     */
    void ProcessInput(Camera &camera) noexcept;

    /**
     * @brief Renders a single frame, including the grid, links, nodes, and UI.
     *
     * @param camera The camera used for rendering the scene.
     * @param callbacks The callbacks for rendering different parts of the
     * frame.
     *
     * @return void
     */
    void RenderFrame(const Camera &camera, const FrameCallbacks &callbacks);

    /**
     * @brief Runs the main rendering loop, processing input and
     *        rendering frames until the window is closed.
     *
     * @param camera The camera used for rendering the scene.
     * @param callbacks The callbacks for rendering different parts of the
     * frame.
     *
     * @return
     */
    void Run(Camera &camera, const FrameCallbacks &callbacks);

    [[nodiscard]] float DpiScale() const noexcept;

    [[nodiscard]] utils::WrappedVector2 ScreenSize() const noexcept;

   private:
    void DrawGrid(const Camera &camera) const;

    bool window_ready_ = false;
    static constexpr float kGridSpacing = 50.0F;
};

}  // namespace render
