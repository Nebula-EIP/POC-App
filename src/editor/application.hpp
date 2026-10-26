/**
 * @file application.hpp
 * @brief Declaration of the Application class, which manages the main loop,
 *        window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 24-10-2026
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
#include "project/nebula_file.hpp"
#include "render/camera.hpp"
#include "render/renderer.hpp"
#include "ui/component_adapter.hpp"
#include "ui/connection_request.hpp"
#include "ui/node_canvas.hpp"
#include "ui/node_clipboard.hpp"
#include "ui/node_picker.hpp"
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
    void OpenNodePicker(utils::WrappedVector2 screen_position,
                        utils::WrappedVector2 world_spawn);
    /// @return The created node, or nothing when it could not be created.
    std::optional<core::NodeId> CreateNode(core::NodeType type,
                                           utils::WrappedVector2 position);
    core::NodeId CreateNodeFromConfiguration(
        core::NodeType type, const core::capa::NodeConfiguration &config,
        std::string title, utils::WrappedVector2 position);
    [[nodiscard]] utils::WrappedVector2 FreeSpawnPosition(
        utils::WrappedVector2 position) const;

    // Editing
    /**
     * @brief Handles a connection request emitted by the node canvas.
     */
    void HandleConnectionRequest();

    /**
     * @brief Opens the node picker in connection mode when a link drag was
     * released on empty canvas, on the nodes that can be linked to the
     * dragged pin. The ghost link stays drawn while the picker is open.
     */
    void HandleLinkDrop();

    /**
     * @brief Creates the node of a picked connection entry at the drop point
     * and links it to the dragged pin. When the link is refused, the node is
     * removed again and the error is shown.
     */
    void CreateLinkedNode(const ui::NodePickerEntry &entry,
                          const ui::LinkDrop &drop);

    /**
     * @brief Shows the error of a refused connection and highlights its pin,
     * and forgets the selected link when the connection replaced it.
     */
    void ReportConnectionOutcome(const ui::ConnectionOutcome &outcome);

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

    /// Name of a type of the given module (type ids are given per module).
    [[nodiscard]] std::string TypeName(core::ModuleId module_id,
                                       core::DataType type);

    /// Displayed name of a module, the top folder of the node picker.
    [[nodiscard]] std::string ModuleName(core::ModuleId module_id) const;

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

    /**
     * @brief Create a new project, clearing the current graph
     */
    void NewProject();

    /**
     * @brief Open a saved project from a .nebula file,
     *        clearing the current graph
     */
    void OpenProject();

    /**
     * @brief Save current project to a .nebula file,
     *        optionally asking for a new file name if save_as is true
     *
     * @return true if the project was saved successfully,
     *         false if the user cancelled the operation
     */
    bool SaveProject(bool save_as);

    /**
     * @brief Ask to save unsaved changes before discarding them
     *
     * @return true if the user confirmed to discard changes,
     *         false if they cancelled
     */
    bool ConfirmDiscardChanges();

    [[nodiscard]] bool HasUnsavedChanges() const;
    [[nodiscard]] project::NebulaDocument BuildDocument() const;
    [[nodiscard]] std::string Snapshot() const;

    /**
     * @brief Clear the graph
     */
    void ClearGraph();

    std::filesystem::path module_folder_;
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
    /// Link dropped on empty canvas, while the picker offers the nodes to
    /// link to it. Its ghost link is drawn until a pick or a cancel.
    std::optional<ui::LinkDrop> link_drop_;
    actions::ActionManager action_manager_;

    std::filesystem::path project_path_;
    project::NebulaMetadata project_metadata_;
    std::string saved_snapshot_;
};

}  // namespace editor