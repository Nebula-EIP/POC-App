/**
 * @file application.hpp
 * @brief Declaration of the Application class, which manages the main loop,
 *        window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 12-10-2026
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "actions/action_manager.hpp"
#include "graph/graph.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/loader.hpp"
#include "modules/module.hpp"
#include "render/camera.hpp"
#include "render/renderer.hpp"
#include "ui/context_menu.hpp"
#include "ui/node_clipboard.hpp"
#include "ui/notification_stack.hpp"
#include "ui/top_bar.hpp"
#include "utils/raylib_wrapper.hpp"

namespace editor {

/**
 * @brief Coordinates the editor window, graph editing, rendering, and module
 * loading lifecycle.
 */
class Application {
   public:
    /**
     * @brief Creates an editor application using the given module folder.
     *
     * @param module_folder The folder containing loadable module libraries.
     */
    explicit Application(std::filesystem::path module_folder);

    /**
     * @brief Releases the editor application resources.
     */
    ~Application() = default;

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(Application &&) = delete;

    /**
     * @brief Runs the editor until the window is closed or quit is requested.
     *
     * @return The application exit status.
     */
    int Run();

   private:
    // Startup
    /**
     * @brief Draws the temporary frame shown while startup is in progress.
     */
    void DrawInitialFrame();

    /**
     * @brief Loads and registers all supported modules from the module folder.
     */
    void LoadModules();

    /**
     * @brief Registers the menus and actions used by the editor.
     */
    void BuildMenus();

    /**
     * @brief Centers the camera on the nodes currently displayed.
     */
    void RecenterView();

    /**
     * @brief Restores the camera zoom to its default level.
     */
    void ResetZoom();

    /**
     * @brief Creates a graph node and its editor view from module metadata.
     *
     * @param type The module node type to create.
     * @param config The node pins and default properties.
     * @param position The initial node position in world coordinates.
     */
    void CreateNodeFromConfiguration(
        core::NodeType type, const core::capa::NodeConfiguration &config,
        utils::WrappedVector2 position);

    // Editing
    /**
     * @brief Handles a connection request emitted by the node canvas.
     */
    void HandleConnectionRequest();

    /**
     * @brief Selects the connection under the given world-space position.
     *
     * @param world The position in world coordinates.
     */
    void SelectConnectionAt(utils::WrappedVector2 world);

    /**
     * @brief Deletes the current node or connection selection.
     */
    void DeleteSelection();

    /**
     * @brief Duplicates the currently selected nodes.
     */
    void DuplicateSelection();

    /**
     * @brief Copies the current selection to the clipboard.
     */
    void CopySelection();

    /**
     * @brief Copies the current selection and removes it from the graph.
     */
    void CutSelection();

    /**
     * @brief Pastes clipboard nodes near the given world-space anchor.
     *
     * @param world_anchor The position around which pasted nodes are placed.
     */
    void PasteClipboard(utils::WrappedVector2 world_anchor);

    /**
     * @brief Gets the display name associated with a data type.
     *
     * @param type The data type whose name is requested.
     *
     * @return The display name of the type.
     */
    [[nodiscard]] std::string TypeName(core::DataType type) const;

    // Feedback
    /**
     * @brief Displays an error message through the editor notification system.
     *
     * @param message The error message to display.
     */
    void ShowError(std::string message);

    // Render
    /**
     * @brief Processes input for menus, the canvas, and editor actions.
     */
    void ProcessInput();

    /**
     * @brief Renders one complete editor frame.
     */
    void DrawFrame();

    /**
     * @brief Draws the graph connections, including selected and ghost links.
     *
     * @param camera The camera used to transform graph coordinates.
     */
    void DrawLinks(const render::Camera &camera);

    /**
     * @brief Computes the default position for a newly created node.
     *
     * @return The node spawn position in world coordinates.
     */
    utils::WrappedVector2 SpawnPosition() const;

    /**
     * @brief Computes the center of the canvas below the top bar.
     *
     * @return The canvas center in screen coordinates.
     */
    utils::WrappedVector2 CanvasCenter() const;

    std::filesystem::path module_folder_;
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
    ui::ContextMenu context_menu_;
    actions::ActionManager action_manager_;
};

}  // namespace editor