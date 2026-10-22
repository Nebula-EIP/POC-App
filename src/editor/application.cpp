/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 22-10-2026
 */

#include "application.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ui/connection_request.hpp"
#include "ui/property_change.hpp"
#include "utils/logger.hpp"

namespace editor {

namespace {
constexpr int kDefaultWidth = 800;
constexpr int kDefaultHeight = 600;
constexpr float kNodeHalfWidth = 50.0F;
constexpr float kNodeHalfHeight = 25.0F;
constexpr int kLoadingTextPosition = 20;
constexpr int kLoadingTextSize = 20;
constexpr float kSpacing = 2.0F;
constexpr utils::WrappedColor kConnectionNormal = {150, 150, 150, 255};
constexpr utils::WrappedColor kConnectionSelected = {255, 255, 255, 255};
constexpr utils::WrappedColor kConnectionGhost = {150, 150, 150, 128};
constexpr float kLinkPickTolerance = 8.0F;
// World offset of a duplicated node from its original.
constexpr float kDuplicateOffset = 30.0F;

bool HitTestBezier(utils::WrappedVector2 point, utils::WrappedVector2 start,
                   utils::WrappedVector2 end, float thickness) {
    const float kOffset = std::max(std::abs(end.x_ - start.x_) / 2.0f, 40.0f);
    utils::WrappedVector2 p0 = start;
    utils::WrappedVector2 p1 = {start.x_ + kOffset, start.y_};
    utils::WrappedVector2 p2 = {end.x_ - kOffset, end.y_};
    utils::WrappedVector2 p3 = end;

    utils::WrappedVector2 last_p = p0;
    const int kSegments = 24;
    for (int i = 1; i <= kSegments; ++i) {
        float t = static_cast<float>(i) / kSegments;
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        utils::WrappedVector2 p;
        p.x_ =
            uuu * p0.x_ + 3 * uu * t * p1.x_ + 3 * u * tt * p2.x_ + ttt * p3.x_;
        p.y_ =
            uuu * p0.y_ + 3 * uu * t * p1.y_ + 3 * u * tt * p2.y_ + ttt * p3.y_;

        if (utils::CheckCollisionPointLineWrapped(
                point, last_p, p, static_cast<int>(thickness))) {
            return true;
        }
        last_p = p;
    }
    return false;
}

}  // namespace

Application::Application(std::filesystem::path module_path)
    : module_folder_(std::move(module_path)),
      renderer_(kDefaultWidth, kDefaultHeight, "Nebula") {}

int Application::Run() {
    DrawInitialFrame();

    LoadModules();
    BuildMenus();

    while (!renderer_.ShouldClose() && !should_quit_) {
        ProcessInput();
        DrawFrame();
    }

    return 0;
}

void Application::DrawInitialFrame() {
    const utils::WrappedVector2 kScreenSize = renderer_.ScreenSize();
    camera_.SetViewport(kScreenSize.x_, kScreenSize.y_);

    renderer_.RenderFrame(
        camera_, render::FrameCallbacks{
                     .draw_links_ = {},
                     .draw_ui_ =
                         []() {
                             utils::DrawTextExWrapped(
                                 "Loading module...", kLoadingTextPosition,
                                 kLoadingTextPosition, kLoadingTextSize,
                                 kSpacing, utils::kDarkgray);
                         },
                     .draw_node_content_ = {},
                 });
}

void Application::LoadModules() {
    std::error_code error;
    if (!std::filesystem::is_directory(module_folder_, error) || error) {
        throw std::runtime_error("Module folder is missing or invalid: " +
                                 module_folder_.string());
    }

    std::vector<std::filesystem::path> module_paths;
    for (const auto &entry :
         std::filesystem::directory_iterator(module_folder_, error)) {
        if (error) {
            throw std::runtime_error("Unable to read module folder '" +
                                     module_folder_.string() +
                                     "': " + error.message());
        }
        if (!entry.is_regular_file(error) || error) {
            continue;
        }
        const std::string kExtension = entry.path().extension().string();
#if defined(_WIN32)
        if (kExtension == ".dll") {
#else
        if (kExtension == ".so") {
#endif
            module_paths.push_back(entry.path());
        }
    }
    if (error) {
        throw std::runtime_error("Unable to read module folder '" +
                                 module_folder_.string() +
                                 "': " + error.message());
    }
    std::sort(module_paths.begin(), module_paths.end());
    if (module_paths.empty()) {
        throw std::runtime_error("No module library was found in folder: " +
                                 module_folder_.string());
    }

    for (const auto &module_path : module_paths) {
        try {
            const core::ModuleId kId = loader_.Load(module_path);
            if (module_ == nullptr) {
                module_id_ = kId;
                module_ = loader_.Module(kId);
            }
        } catch (const std::exception &error) {
            throw std::runtime_error("Failed to load module '" +
                                     module_path.string() +
                                     "': " + error.what());
        }
    }

    if (module_ == nullptr) {
        throw std::runtime_error("No module could be resolved from folder: " +
                                 module_folder_.string());
    }

    utils::Logger::GetInstance().Log(
        utils::LogLevel::kInfo, std::source_location::current(),
        "Loaded {} module(s) from {}", loader_.Size(), module_folder_.string());

    core::DataType type_id = 1;
    while (module_->Types()->RegisterType(type_id) != nullptr) {
        ++type_id;
    }
    core::NodeType node_type_id = 1;
    while (module_->Nodes()->RegisterNode(node_type_id) != nullptr) {
        ++node_type_id;
    }

    // The fields of the nodes report their changes here; they are applied to
    // the graph once the fields are done with the frame's input.
    node_renderer_ = module_->Capability<core::capa::IRendererCapability>();
    if (node_renderer_ != nullptr) {
        node_renderer_->SetPropertyChangeHandler(
            [this](core::NodeId node_id, const core::PropertyMap &changes) {
                pending_property_changes_.emplace_back(node_id, changes);
            });
    }

    const auto kAvailableNodes = module_->Nodes()->GetAvailableNodes();
    for (std::size_t index = 0; index < kAvailableNodes.size(); ++index) {
        const auto &metadata = kAvailableNodes[index];
        CreateNodeFromConfiguration(
            metadata.type_,
            module_->Nodes()->GetNodeConfiguration(metadata.type_),
            {40.0F + 220.0F * static_cast<float>(index % 3),
             40.0F + 150.0F * static_cast<float>(index / 3)});
    }
}

void Application::BuildMenus() {
    // Register actions
    actions::ActionManager &am = action_manager_;

    am.RegisterAction({"app.quit",
                       "Quit",
                       {utils::WrappedKey::kNone, false, false, false},
                       [this] { should_quit_ = true; }});

    am.RegisterAction({"view.recenter",
                       "Recenter",
                       {utils::WrappedKey::kNone, false, false, false},
                       [this] { RecenterView(); }});

    am.RegisterAction({"view.reset_zoom",
                       "Zoom 100 %",
                       {utils::WrappedKey::kNone, false, false, false},
                       [this] { ResetZoom(); }});

    am.RegisterAction(
        {"node.add_menu",
         "Add Node",
         {utils::WrappedKey::kA, false, true, false},  // Shift+A
         [this] {
             context_menu_.Clear();
             // Spawn where the menu was opened, not where the item is
             // clicked.
             const utils::WrappedVector2 kSpawnWorld =
                 camera_.ScreenToWorld(cursor_position_);
             if (module_ != nullptr) {
                 // Populate node types
                 for (const auto &node_meta :
                      module_->Nodes()->GetAvailableNodes()) {
                     const core::NodeType kType = node_meta.type_;
                     const std::string kName = node_meta.name_;
                     context_menu_.AddAction(
                         kName, [this, kType, kSpawnWorld]() {
                             CreateNodeFromConfiguration(
                                 kType,
                                 module_->Nodes()->GetNodeConfiguration(kType),
                                 kSpawnWorld);
                         });
                 }
             }
             context_menu_.OpenAt(cursor_position_, renderer_.ScreenSize().x_,
                                  renderer_.ScreenSize().y_);
         }});

    // Build top bar
    top_bar_.AddMenu("File")
        .AddAction("Nouveau")
        .AddAction("Ouvrir...")
        .AddAction("Enregistrer")
        .AddSeparator()
        .AddAction(am.GetAction("app.quit")->display_name_,
                   am.GetAction("app.quit")->callback_,
                   am.GetAction("app.quit")->shortcut_.ToString());

    top_bar_.AddMenu("Édition")
        .AddAction("Couper (Ctrl+X)", [this] { CutSelection(); })
        .AddAction("Copier (Ctrl+C)", [this] { CopySelection(); })
        .AddAction("Coller (Ctrl+V)",
                   [this] {
                       // From the menu the cursor is on the menu: paste in
                       // the middle of the view instead.
                       PasteClipboard(camera_.ScreenToWorld(CanvasCenter()));
                   })
        .AddSeparator()
        .AddAction("Dupliquer (Ctrl+D)", [this] { DuplicateSelection(); })
        .AddAction("Supprimer (Suppr)", [this] { DeleteSelection(); });

    top_bar_.AddMenu("Affichage")
        .AddAction(am.GetAction("view.recenter")->display_name_,
                   am.GetAction("view.recenter")->callback_,
                   am.GetAction("view.recenter")->shortcut_.ToString())
        .AddAction(am.GetAction("view.reset_zoom")->display_name_,
                   am.GetAction("view.reset_zoom")->callback_,
                   am.GetAction("view.reset_zoom")->shortcut_.ToString());

    top_bar_.AddMenu("Node").AddAction(
        am.GetAction("node.add_menu")->display_name_,
        am.GetAction("node.add_menu")->callback_,
        am.GetAction("node.add_menu")->shortcut_.ToString());
}

void Application::RecenterView() {
    utils::WrappedVector2 center{0.0F, 0.0F};

    const auto &views = renderer_.node_canvas_.Views();
    if (!views.empty()) {
        utils::WrappedVector2 min{std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max()};
        utils::WrappedVector2 max{std::numeric_limits<float>::lowest(),
                                  std::numeric_limits<float>::lowest()};
        for (const auto &[id, view] : views) {
            (void)id;
            const utils::WrappedRectangle &bounds = view->Bounds();
            min.x_ = std::min(min.x_, bounds.x_);
            min.y_ = std::min(min.y_, bounds.y_);
            max.x_ = std::max(max.x_, bounds.x_ + bounds.width_);
            max.y_ = std::max(max.y_, bounds.y_ + bounds.height_);
        }
        center = {(min.x_ + max.x_) / 2.0F, (min.y_ + max.y_) / 2.0F};
    }
    camera_.CenterOn(center, CanvasCenter());
}

void Application::ResetZoom() { camera_.SetZoom(1.0F, CanvasCenter()); }

utils::WrappedVector2 Application::CanvasCenter() const {
    const utils::WrappedVector2 kScreen = renderer_.ScreenSize();
    const float kTop = top_bar_.Height();
    return {kScreen.x_ / 2.0F, kTop + (kScreen.y_ - kTop) / 2.0F};
}

void Application::CreateNodeFromConfiguration(
    core::NodeType type, const core::capa::NodeConfiguration &config,
    utils::WrappedVector2 position) {
    core::Node &node = graph_.CreateNode(type);

    for (const auto &pin : config.input_pins_) {
        graph_.AddInputPin(node.Id(), pin.name_, pin.type_);
    }

    for (const auto &pin : config.output_pins_) {
        graph_.AddOutputPin(node.Id(), pin.name_, pin.type_);
    }

    // Keep the ids chosen by the module: its fields read them back by id.
    for (const auto &[property_id, property] : config.default_properties_) {
        node.SetProperty(property_id, property);
    }
    std::string title = "Node";
    if (module_ != nullptr) {
        for (const auto &metadata : module_->Nodes()->GetAvailableNodes()) {
            if (metadata.type_ == type) {
                title = metadata.name_;
                break;
            }
        }
    }
    renderer_.node_canvas_.AddNode(node, std::move(title), position);
}

utils::WrappedVector2 Application::SpawnPosition() const {
    return utils::WrappedVector2{cursor_position_.x_ - kNodeHalfWidth,
                                 cursor_position_.y_ - kNodeHalfHeight};
}

void Application::ProcessInput() {
    cursor_position_ = utils::GetCursorPositionWrapped();

    // Context menu goes first. If it consumes input, others shouldn't.
    ui::ContextMenuInput context_menu_input{
        .cursor_ = cursor_position_,
        .left_pressed_ = utils::IsLeftClicked(),
        .right_pressed_ =
            utils::IsRightClicked(),  // Wait, is there IsRightClicked?
        .escape_pressed_ =
            utils::IsKeyPressedWrapped(utils::WrappedKey::kEscape),
        .screen_width_ = renderer_.ScreenSize().x_,
        .screen_height_ = renderer_.ScreenSize().y_};
    const bool kContextCaptured = context_menu_.Update(context_menu_input);

    // The top bar goes second.
    const bool kTopBarCaptured = top_bar_.Update(ui::TopBar::ReadInput());
    const bool kPointerCaptured = kContextCaptured || kTopBarCaptured;

    // The fields of the selected node go third: a click on a field (or an
    // open list hanging below the node) must not reach the canvas.
    RefreshNodeFields();
    ui::ComponentInput field_input =
        ui::ComponentInput::Read(camera_.ScreenToWorld(cursor_position_));
    if (kPointerCaptured) {
        field_input.left_pressed_ = false;
    }
    const std::optional<core::NodeId> kEditedBefore = EditedNode();
    const bool kFieldsCaptured = UpdateNodeFields(field_input);

    renderer_.ProcessInput(camera_, kPointerCaptured || kFieldsCaptured);

    // A click that selects a node can also reach its fields right away.
    const std::optional<core::NodeId> kEditedAfter = EditedNode();
    if (!kFieldsCaptured && field_input.left_pressed_ && kEditedAfter &&
        kEditedAfter != kEditedBefore) {
        ui::ComponentInput click_only;
        click_only.cursor_ = field_input.cursor_;
        click_only.left_pressed_ = true;
        UpdateNodeFields(click_only);
    }

    // While typing in a field, keys belong to the field.
    const bool kTyping = node_fields_.IsEditingText();
    if (!kTyping) {
        action_manager_.ProcessShortcuts();
    }

    const bool kCanvasClicked =
        !kPointerCaptured && !kFieldsCaptured && utils::IsLeftClicked();

    if (kCanvasClicked) {
        for (const auto &[id, view] : renderer_.node_canvas_.Views()) {
            (void)id;
            view->ClearErrorPin();
        }
        SelectConnectionAt(camera_.ScreenToWorld(cursor_position_));
    }

    HandleConnectionRequest();
    notifications_.Prune(utils::GetTimeWrapped());

    if (kTyping) {
        return;
    }

    const bool kControlDown =
        utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl) ||
        utils::IsKeyDownWrapped(utils::WrappedKey::kRightControl);
    if (utils::IsKeyPressedWrapped(utils::WrappedKey::kDelete)) {
        DeleteSelection();
    }
    if (kControlDown && utils::IsKeyPressedWrapped(utils::WrappedKey::kD)) {
        DuplicateSelection();
    }
    if (kControlDown && utils::IsKeyPressedWrapped(utils::WrappedKey::kC)) {
        CopySelection();
    }
    if (kControlDown && utils::IsKeyPressedWrapped(utils::WrappedKey::kX)) {
        CutSelection();
    }
    if (kControlDown && utils::IsKeyPressedWrapped(utils::WrappedKey::kV)) {
        PasteClipboard(camera_.ScreenToWorld(cursor_position_));
    }

    if (utils::IsKeyPressedWrapped(utils::WrappedKey::kH)) {
        if (utils::IsCursorHiddenWrapped()) {
            utils::ShowCursorWrapped();
        } else {
            utils::HideCursorWrapped();
        }
    }
}

std::optional<core::NodeId> Application::EditedNode() const {
    const std::vector<core::NodeId> kSelected =
        renderer_.node_canvas_.SelectedNodes();
    if (kSelected.size() != 1) {
        return std::nullopt;
    }
    return kSelected.front();
}

void Application::RefreshNodeFields() {
    // Fields of removed nodes are forgotten.
    std::erase_if(node_components_, [this](const auto &entry) {
        return !renderer_.node_canvas_.Views().contains(entry.first);
    });
    if (const auto kFocused = node_fields_.FocusedNode();
        kFocused && !renderer_.node_canvas_.Views().contains(*kFocused)) {
        node_fields_.ClearNode(*kFocused);
    }

    for (const auto &[id, view] : renderer_.node_canvas_.Views()) {
        core::capa::ComponentList components;
        const core::Node *node = graph_.GetNode(id);
        if (node != nullptr && node_renderer_ != nullptr &&
            node_renderer_->SupportsNodeType(node->Type())) {
            try {
                components = node_renderer_->GetNodeComponents(
                    id, node->Type(), node->Properties());
            } catch (const std::exception &error) {
                utils::Logger::GetInstance().Log(
                    utils::LogLevel::kError, std::source_location::current(),
                    "Fields of node {} unavailable: {}", id, error.what());
            }
        }
        view->SetContentSize(ui::ComponentAdapter::PreferredSize(components));
        node_components_[id] = std::move(components);
    }
}

bool Application::UpdateNodeFields(const ui::ComponentInput &input) {
    const std::optional<core::NodeId> kEdited = EditedNode();
    if (!kEdited) {
        // Nothing is edited any more (selection cleared or several nodes).
        node_fields_.CancelEdit();
        return false;
    }
    const auto &views = renderer_.node_canvas_.Views();
    const auto kView = views.find(*kEdited);
    if (kView == views.end()) {
        return false;
    }
    const bool kCaptured =
        node_fields_.Update(*kEdited, node_components_[*kEdited],
                            kView->second->ContentBounds(), input);
    for (std::string &message : node_fields_.TakeMessages()) {
        ShowError(std::move(message));
    }
    ApplyPropertyChanges();
    return kCaptured;
}

void Application::ApplyPropertyChanges() {
    if (pending_property_changes_.empty()) {
        return;
    }
    // Take them: applying may not add new ones, but be safe.
    auto changes = std::move(pending_property_changes_);
    pending_property_changes_.clear();

    const auto kTitle = [this](core::NodeId node_id) {
        const auto &views = renderer_.node_canvas_.Views();
        const auto kView = views.find(node_id);
        return kView == views.end() ? std::string{"?"} : kView->second->Title();
    };
    for (const auto &[node_id, properties] : changes) {
        const ui::PropertyChangeOutcome kOutcome = ui::ApplyPropertyChange(
            graph_, module_ != nullptr ? module_->Nodes() : nullptr, node_id,
            properties, [this](core::DataType type) { return TypeName(type); },
            kTitle);

        const auto &views = renderer_.node_canvas_.Views();
        const auto kView = views.find(node_id);
        if (kOutcome.applied_) {
            const core::Node *node = graph_.GetNode(node_id);
            if (!kOutcome.pin_changes_.empty() && node != nullptr &&
                kView != views.end()) {
                kView->second->Refresh(*node);
            }
            continue;
        }

        std::string message = kOutcome.error_;
        if (kOutcome.blocked_pin_ && kView != views.end()) {
            kView->second->SetErrorPin(kOutcome.blocked_pin_->pin_id_,
                                       kOutcome.blocked_pin_->is_input_);
        }
        if (!kOutcome.blocking_connections_.empty()) {
            // Select only the link, so Suppr removes it and nothing else.
            renderer_.node_canvas_.SetSelection({});
            node_fields_.CancelEdit();
            selected_connection_ = kOutcome.blocking_connections_.front();
            message += kOutcome.blocking_connections_.size() == 1
                           ? " (sélectionnée : Suppr)"
                           : " (la première est sélectionnée : Suppr)";
        }
        ShowError(std::move(message));
    }
    // Show the new values this frame already.
    RefreshNodeFields();
}

void Application::HandleConnectionRequest() {
    const auto kPending = renderer_.node_canvas_.PopPendingConnectionRequest();
    if (!kPending) {
        return;
    }

    const ui::ConnectionOutcome kOutcome = ui::RequestConnection(
        graph_, kPending->first, kPending->second,
        [this](core::DataType type) { return TypeName(type); });
    // A replaced link no longer exists: it cannot stay selected.
    if (selected_connection_ &&
        std::find(kOutcome.replaced_.begin(), kOutcome.replaced_.end(),
                  *selected_connection_) != kOutcome.replaced_.end()) {
        selected_connection_.reset();
    }
    if (kOutcome.Succeeded() || kOutcome.error_.empty()) {
        return;
    }

    if (kOutcome.faulty_pin_) {
        const auto &views = renderer_.node_canvas_.Views();
        if (const auto kView = views.find(kOutcome.faulty_pin_->node_id_);
            kView != views.end()) {
            kView->second->SetErrorPin(
                kOutcome.faulty_pin_->pin_id_,
                kOutcome.faulty_pin_->part_ == ui::HitPart::kInputPin);
        }
    }
    ShowError(kOutcome.error_);
}

void Application::SelectConnectionAt(utils::WrappedVector2 world) {
    selected_connection_.reset();

    // A node (or one of its pins) above a link takes the click.
    if (renderer_.node_canvas_.HitTest(world)) {
        return;
    }

    const float kTolerance = kLinkPickTolerance / camera_.Zoom();
    const auto &views = renderer_.node_canvas_.Views();
    for (const auto &[id, connection] : graph_.GetAllConnections()) {
        if (!views.contains(connection.from_node_) ||
            !views.contains(connection.to_node_)) {
            continue;
        }
        const auto kStartPos = views.at(connection.from_node_)
                                   ->GetPinPosition(connection.out_pin_, false);
        const auto kEndPos = views.at(connection.to_node_)
                                 ->GetPinPosition(connection.in_pin_, true);
        if (kStartPos && kEndPos &&
            HitTestBezier(world, *kStartPos, *kEndPos, kTolerance)) {
            selected_connection_ = id;
            return;
        }
    }
}

void Application::DeleteSelection() {
    if (selected_connection_) {
        graph_.Disconnect(*selected_connection_);
        selected_connection_.reset();
    }

    const std::vector<core::NodeId> kSelected =
        renderer_.node_canvas_.SelectedNodes();
    if (kSelected.empty()) {
        return;
    }
    // Graph::RemoveNode also removes every connection of the node.
    for (const core::NodeId kId : kSelected) {
        graph_.RemoveNode(kId);
    }
    renderer_.node_canvas_.RemoveMissingNodes(graph_);
}

void Application::DuplicateSelection() {
    const std::vector<core::NodeId> kSelected =
        renderer_.node_canvas_.SelectedNodes();
    if (kSelected.empty()) {
        return;
    }

    std::vector<core::NodeId> copies;
    copies.reserve(kSelected.size());
    for (const core::NodeId kId : kSelected) {
        const auto &views = renderer_.node_canvas_.Views();
        const auto kView = views.find(kId);
        if (kView == views.end()) {
            continue;
        }
        std::string title = kView->second->Title();
        const utils::WrappedRectangle kBounds = kView->second->Bounds();

        // Same type, pins and properties; no connection.
        const core::Node &copy = graph_.DuplicateNode(kId);
        renderer_.node_canvas_.AddNode(
            copy, std::move(title),
            {kBounds.x_ + kDuplicateOffset, kBounds.y_ + kDuplicateOffset});
        copies.push_back(copy.Id());
    }

    // The copies replace the originals in the selection, so they can be
    // moved right away.
    renderer_.node_canvas_.SetSelection(copies);
    selected_connection_.reset();
}

void Application::CopySelection() {
    std::vector<ui::PlacedNode> nodes;
    const auto &views = renderer_.node_canvas_.Views();
    for (const core::NodeId kId : renderer_.node_canvas_.SelectedNodes()) {
        const auto kView = views.find(kId);
        if (kView == views.end()) {
            continue;
        }
        nodes.push_back(ui::PlacedNode{
            .id_ = kId,
            .title_ = kView->second->Title(),
            .position_ = {kView->second->Bounds().x_,
                          kView->second->Bounds().y_},
        });
    }
    clipboard_.Copy(graph_, nodes);
}

void Application::CutSelection() {
    // Without a selected node there is nothing to cut: keep the clipboard.
    if (renderer_.node_canvas_.SelectedNodes().empty()) {
        return;
    }
    CopySelection();
    DeleteSelection();
}

void Application::PasteClipboard(utils::WrappedVector2 world_anchor) {
    if (clipboard_.Empty()) {
        return;
    }

    std::vector<core::NodeId> pasted_ids;
    for (ui::PlacedNode &pasted : clipboard_.Paste(graph_, world_anchor)) {
        const core::Node *node = graph_.GetNode(pasted.id_);
        if (node == nullptr) {
            continue;
        }
        renderer_.node_canvas_.AddNode(*node, std::move(pasted.title_),
                                       pasted.position_);
        pasted_ids.push_back(pasted.id_);
    }

    // The pasted nodes become the selection, so they can be moved right away.
    renderer_.node_canvas_.SetSelection(pasted_ids);
    selected_connection_.reset();
}

std::string Application::TypeName(core::DataType type) const {
    if (module_ == nullptr || module_->Types() == nullptr) {
        return {};
    }
    return std::string(module_->Types()->TypeName(type));
}

void Application::ShowError(std::string message) {
    notifications_.Push(std::move(message), utils::GetTimeWrapped());
}

void Application::DrawLinks(const render::Camera &camera) {
    (void)camera;  // Unused but passed by callback
    const auto &views = renderer_.node_canvas_.Views();
    for (const auto &[id, connection] : graph_.GetAllConnections()) {
        if (!views.contains(connection.from_node_) ||
            !views.contains(connection.to_node_)) {
            continue;
        }
        const auto kStartPos = views.at(connection.from_node_)
                                   ->GetPinPosition(connection.out_pin_, false);
        const auto kEndPos = views.at(connection.to_node_)
                                 ->GetPinPosition(connection.in_pin_, true);

        if (kStartPos && kEndPos) {
            const bool kIsSelected =
                selected_connection_ && *selected_connection_ == id;
            const auto kColor =
                kIsSelected ? kConnectionSelected : kConnectionNormal;
            utils::DrawLineBezierWrapped(*kStartPos, *kEndPos, 3.0f, kColor);
        }
    }

    if (auto ghost_link = renderer_.node_canvas_.GetConnectionDragLine()) {
        utils::DrawLineBezierWrapped(ghost_link->first, ghost_link->second,
                                     3.0f, kConnectionGhost);
    }
}

void Application::DrawFrame() {
    renderer_.RenderFrame(
        camera_, render::FrameCallbacks{
                     .draw_links_ =
                         [this](const render::Camera &cam) { DrawLinks(cam); },
                     // Screen space, after the canvas: pan and zoom never
                     // move the bar.
                     .draw_ui_ =
                         [this]() {
                             notifications_.Draw(renderer_.ScreenSize(),
                                                 utils::GetTimeWrapped());
                             top_bar_.Draw();
                             context_menu_.Draw();
                         },
                     // World space, right after each node.
                     .draw_node_content_ =
                         [this](const ui::NodeView &view) {
                             const auto kComponents =
                                 node_components_.find(view.NodeId());
                             if (kComponents == node_components_.end()) {
                                 return;
                             }
                             node_fields_.Draw(view.NodeId(),
                                               kComponents->second,
                                               view.ContentBounds(),
                                               EditedNode() == view.NodeId());
                         }});
}

}  // namespace editor