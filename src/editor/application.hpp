/**
 * @file application.hpp
 * @brief Declaration of the Application class, which manages the main loop,
 *        window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 21-09-2026
 */

#pragma once

#include <raylib.h>

#include <filesystem>
#include <memory>
#include <vector>

#include "graph/graph.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/loader.hpp"
#include "modules/module.hpp"

namespace editor {

class Application {
   public:
    explicit Application(std::filesystem::path module_path);
    ~Application();

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
        Vector2 position);

    // Render
    void ProcessInput();
    void DrawFrame();

    Vector2 SpawnPosition() const;

    std::filesystem::path module_path_;
    bool window_ready_ = false;
    bool should_quit_ = false;
    Vector2 cursor_position_{0.0F, 0.0F};

    core::ModuleId module_id_ = 0;
    core::ModuleLoader loader_;

    core::IModule *module_ = nullptr;
    core::Graph graph_;
    // Tob bar variable
};

}  // namespace editor
