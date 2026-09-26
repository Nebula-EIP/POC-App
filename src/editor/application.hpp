/**
 * @file application.hpp
 * @brief Declaration of the Application class, which manages the main loop,
 *        window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 26-09-2026
 */

#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "graph/graph.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/loader.hpp"
#include "modules/module.hpp"
#include "render/camera.hpp"
#include "render/renderer.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor {

class Application {
   public:
    explicit Application(std::filesystem::path module_path);
    ~Application() = default;

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(Application &&) = delete;

    int Run();

   private:
    // Startup
    void DrawInitialFrame();
    void LoadModule();
    void BuildMenus();

    void CreateNodeFromConfiguration(
        core::NodeType type, const core::capa::NodeConfiguration &config,
        utils::WrappedVector2 position);

    // Render
    void ProcessInput();
    void DrawFrame();

    utils::WrappedVector2 SpawnPosition() const;

    std::filesystem::path module_path_;
    bool should_quit_ = false;
    utils::WrappedVector2 cursor_position_{0.0F, 0.0F};

    // renderer_ opens the window and must outlive every module-derived
    // resource, so it is declared (and destroyed last, i.e. first here)
    // before loader_/module_/graph_. graph_ must still be destroyed before
    // loader_ unloads its module and closes the shared library, so it stays
    // declared after loader_/module_id_/module_, as before.
    render::Renderer renderer_;
    render::Camera camera_;

    core::ModuleLoader loader_;
    core::ModuleId module_id_ = 0;

    core::IModule *module_ = nullptr;
    core::Graph graph_;
    // Tob bar variable
};

}  // namespace editor