/**
 * @file application.hpp
 * @brief Declaration of the Application class, which manages the main loop,
 *        window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 09-10-2026
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "graph/graph.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/loader.hpp"
#include "modules/module.hpp"
#include "render/camera.hpp"
#include "render/renderer.hpp"
#include "ui/node_clipboard.hpp"
#include "ui/notification_stack.hpp"
#include "ui/top_bar.hpp"
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
    void RecenterView();
    void ResetZoom();

    void CreateNodeFromConfiguration(
        core::NodeType type, const core::capa::NodeConfiguration &config,
        utils::WrappedVector2 position);

    // Editing
    void HandleConnectionRequest();
    void SelectConnectionAt(utils::WrappedVector2 world);
    void DeleteSelection();
    void DuplicateSelection();
    void CopySelection();
    void PasteClipboard(utils::WrappedVector2 world_anchor);
    [[nodiscard]] std::string TypeName(core::DataType type) const;

    // Feedback
    void ShowError(std::string message);

    // Render
    void ProcessInput();
    void DrawFrame();

    void DrawLinks(const render::Camera &camera);

    utils::WrappedVector2 SpawnPosition() const;
    utils::WrappedVector2 CanvasCenter() const;

    std::filesystem::path module_path_;
    bool should_quit_ = false;
    utils::WrappedVector2 cursor_position_{0.0F, 0.0F};

    render::Renderer renderer_;
    render::Camera camera_;

    core::ModuleLoader loader_;
    core::ModuleId module_id_ = 0;

    core::IModule *module_ = nullptr;
    core::Graph graph_;

    std::optional<core::ConnectionId> selected_connection_;

    ui::NotificationStack notifications_;
    ui::NodeClipboard clipboard_;

    ui::TopBar top_bar_;
};

}  // namespace editor