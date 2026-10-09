/**
 * @file renderer.hpp
 * @brief Declaration of the Renderer class, which manages the rendering loop
 * and drawing of the graph
 *
 * @author Created by NathanBezard
 * @date Created on 26-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 09-10-2026
 */

#pragma once

#include <functional>
#include <string>

#include "camera.hpp"
#include "ui/node_canvas.hpp"
#include "utils/raylib_wrapper.hpp"

namespace render {

/**
 * @brief Struct to hold callbacks for rendering different parts of the frame.
 *
 */
struct FrameCallbacks {
    std::function<void(const Camera &)> draw_links_;
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

    editor::ui::NodeCanvas node_canvas_;

    [[nodiscard]] bool ShouldClose() const noexcept;

    /**
     * @brief Processes user input events and updates the camera and node
     * canvas accordingly.
     *
     * @param camera The camera to be updated based on user input.
     * @param pointer_captured True when an overlay (e.g. the top bar) owns the
     * mouse this frame: the viewport is still updated, but pan, zoom and the
     * node canvas ignore the mouse.
     */
    void ProcessInput(Camera &camera, bool pointer_captured = false) noexcept;

    /**
     * @brief Renders a single frame: grid, links, nodes, then UI, in that
     * order.
     *
     * @param camera The camera used for rendering the scene.
     * @param callbacks The callbacks for rendering the remaining layers.
     */
    void RenderFrame(const Camera &camera, const FrameCallbacks &callbacks);

    /**
     * @brief Runs the main rendering loop, processing input and
     *        rendering frames until the window is closed.
     *
     * @param camera The camera used for rendering the scene.
     * @param callbacks The callbacks for rendering the remaining layers.
     */
    void Run(Camera &camera, const FrameCallbacks &callbacks);

    [[nodiscard]] float DpiScale() const noexcept;

    [[nodiscard]] utils::WrappedVector2 ScreenSize() const noexcept;

   private:
    /**
     * @brief Draws the adaptive grid (see grid.hpp): the coarse level fully
     * opaque, the fine level faded in between, both 1 pixel wide on screen.
     *
     * @param camera The camera the grid is drawn through.
     */
    void DrawGrid(const Camera &camera) const;

    bool window_ready_ = false;
};

}  // namespace render