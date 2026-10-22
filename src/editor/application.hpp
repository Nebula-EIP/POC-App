/**
 * @file application.hpp
 * @brief Declaration of the Application class, which manages the main loop,
 *        window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 11-10-2026
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "actions/action_manager.hpp"
#include "graph/graph.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/renderer_capability.hpp"
#include "modules/loader.hpp"
#include "modules/module.hpp"
#include "node_catalog/node_catalog.hpp"
#include "render/camera.hpp"
#include "render/renderer.hpp"
#include "ui/component_adapter.hpp"
#include "ui/node_clipboard.hpp"
#include "ui/node_picker.hpp"
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

    // Node creation
    void OpenNodePicker(utils::WrappedVector2 screen_position,
                        utils::WrappedVector2 world_spawn);
    void CreateNode(core::NodeType type, utils::WrappedVector2 position);
    void CreateNodeFromConfiguration(
        core::NodeType type, const core::capa::NodeConfiguration &config,
        std::string title, utils::WrappedVector2 position);
    [[nodiscard]] utils::WrappedVector2 FreeSpawnPosition(
        utils::WrappedVector2 position) const;

    // Editing
    void HandleConnectionRequest();

    // Node fields (values and settings)
    /**
     * @brief The node whose fields can be edited: the only selected node.
     *
     */
    [[nodiscard]] std::optional<core::NodeId> EditedNode() const;
    /**
     * @brief Rebuilds the fields of every node from the graph and sizes the
     * nodes.
     *
     */
    void RefreshNodeFields();
    /**
     * @brief Sends this frame's input to the fields of the edited node.
     * @return true when the fields used the click.
     *
     */
    bool UpdateNodeFields(const ui::ComponentInput &input);
    /**
     * @brief Applies the property changes reported by the fields to the graph.
     *
     */
    void ApplyPropertyChanges();
    void SelectConnectionAt(utils::WrappedVector2 world);
    void DeleteSelection();
    void DuplicateSelection();
    void CopySelection();
    void CutSelection();
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
    core::NodeCatalog node_catalog_;
    core::Graph graph_;

    std::optional<core::ConnectionId> selected_connection_;

    core::capa::IRendererCapability *node_renderer_ = nullptr;
    ui::ComponentAdapter node_fields_;
    std::unordered_map<core::NodeId, core::capa::ComponentList>
        node_components_;
    std::vector<std::pair<core::NodeId, core::PropertyMap>>
        pending_property_changes_;

    ui::NotificationStack notifications_;
    ui::NodeClipboard clipboard_;

    ui::TopBar top_bar_;
    ui::NodePicker node_picker_;
    actions::ActionManager action_manager_;
};

}  // namespace editor