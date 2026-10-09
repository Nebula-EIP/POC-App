/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 09-10-2026
 */

#include "application.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ui/connection_request.hpp"
#include "utils/logger.hpp"

namespace editor {

namespace {
constexpr int kDefaultWidth = 800;
constexpr int kDefaultHeight = 600;
constexpr float kNodeHalfWidth = 50.0F;
constexpr float kNodeHalfHeight = 25.0F;
constexpr int kLoadingTextPosition = 20;
constexpr int kLoadingTextSize = 20;
constexpr utils::WrappedColor kConnectionNormal = {150, 150, 150, 255};
constexpr utils::WrappedColor kConnectionSelected = {255, 255, 255, 255};
constexpr utils::WrappedColor kConnectionGhost = {150, 150, 150, 128};
// Screen pixels around a link that still select it.
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
    : module_path_(std::move(module_path)),
      renderer_(kDefaultWidth, kDefaultHeight, "Nebula") {}

int Application::Run() {
    DrawInitialFrame();

    LoadModule();
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
                             utils::DrawTextWrapped(
                                 "Loading module...", kLoadingTextPosition,
                                 kLoadingTextPosition, kLoadingTextSize,
                                 utils::kDarkgray);
                         },
                 });
}

void Application::LoadModule() {
    module_id_ = loader_.Load(module_path_);
    module_ = loader_.Module(module_id_);

    if (module_ == nullptr) {
        throw std::runtime_error("Loaded module " + module_path_.string() +
                                 " could not be resolved by id");
    }

    utils::Logger::GetInstance().Log(utils::LogLevel::kInfo,
                                     std::source_location::current(),
                                     "Loaded module {}", module_path_.string());

    core::DataType type_id = 1;
    while (module_->Types()->RegisterType(type_id) != nullptr) {
        ++type_id;
    }
    core::NodeType node_type_id = 1;
    while (module_->Nodes()->RegisterNode(node_type_id) != nullptr) {
        ++node_type_id;
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

    for (const auto &[property_id, property] : config.default_properties_) {
        (void)property_id;
        node.AddProperty(property);
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

    renderer_.ProcessInput(camera_, kPointerCaptured);

    // Process shortcuts if pointer is not captured by a menu (or maybe always?)
    // The requirement says: "Shortcuts must not trigger unrelated actions while
    // the user is typing in a text field." For now we just process them.
    action_manager_.ProcessShortcuts();

    const bool kCanvasClicked = !kPointerCaptured && utils::IsLeftClicked();

    if (kCanvasClicked) {
        for (const auto &[id, view] : renderer_.node_canvas_.Views()) {
            (void)id;
            view->ClearErrorPin();
        }
        SelectConnectionAt(camera_.ScreenToWorld(cursor_position_));
    }

    HandleConnectionRequest();
    notifications_.Prune(utils::GetTimeWrapped());

    const bool kControlDown =
        utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl) ||
        utils::IsKeyDownWrapped(utils::WrappedKey::kRightControl);
    if (utils::IsKeyPressedWrapped(utils::WrappedKey::kDelete)) {
        DeleteSelection();
    // Check for pending connection requests
    if (auto pending = renderer_.node_canvas_.PopPendingConnectionRequest()) {
        const auto &[start_hit, end_hit] = *pending;
        if (start_hit.part_ == ui::HitPart::kOutputPin &&
            end_hit.part_ == ui::HitPart::kInputPin) {
            try {
                graph_.Connect(start_hit.node_id_, start_hit.pin_id_,
                               end_hit.node_id_, end_hit.pin_id_);
            } catch (const std::exception &) {
                if (auto view =
                        renderer_.node_canvas_.Views().find(end_hit.node_id_);
                    view != renderer_.node_canvas_.Views().end()) {
                    view->second->SetErrorPin(end_hit.pin_id_, true);
                }
                if (auto view =
                        renderer_.node_canvas_.Views().find(start_hit.node_id_);
                    view != renderer_.node_canvas_.Views().end()) {
                    view->second->SetErrorPin(start_hit.pin_id_, false);
                }
            }
        } else if (start_hit.part_ == ui::HitPart::kInputPin &&
                   end_hit.part_ == ui::HitPart::kOutputPin) {
            try {
                graph_.Connect(end_hit.node_id_, end_hit.pin_id_,
                               start_hit.node_id_, start_hit.pin_id_);
            } catch (const std::exception &) {
                if (auto view =
                        renderer_.node_canvas_.Views().find(start_hit.node_id_);
                    view != renderer_.node_canvas_.Views().end()) {
                    view->second->SetErrorPin(start_hit.pin_id_, true);
                }
                if (auto view =
                        renderer_.node_canvas_.Views().find(end_hit.node_id_);
                    view != renderer_.node_canvas_.Views().end()) {
                    view->second->SetErrorPin(end_hit.pin_id_, false);
                }
            }
        }
    }

    // Hit test for connection selection and deletion
    const auto kCursorWorld = camera_.ScreenToWorld(cursor_position_);
    if (kCanvasClicked) {
        hovered_connection_ = std::nullopt;
        const auto &views = renderer_.node_canvas_.Views();
        for (const auto &[id, connection] : graph_.GetAllConnections()) {
            if (!views.contains(connection.from_node_) ||
                !views.contains(connection.to_node_)) {
                continue;
            }
            const auto kStartPos =
                views.at(connection.from_node_)
                    ->GetPinPosition(connection.out_pin_, false);
            const auto kEndPos = views.at(connection.to_node_)
                                     ->GetPinPosition(connection.in_pin_, true);
            if (kStartPos && kEndPos) {
                if (HitTestBezier(kCursorWorld, *kStartPos, *kEndPos, 8.0f)) {
                    hovered_connection_ = id;
                    break;
                }
            }
        }
    }
    if (kControlDown && utils::IsKeyPressedWrapped(utils::WrappedKey::kD)) {
        DuplicateSelection();
    }
    if (kControlDown && utils::IsKeyPressedWrapped(utils::WrappedKey::kC)) {
        CopySelection();
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

void Application::HandleConnectionRequest() {
    const auto kPending = renderer_.node_canvas_.PopPendingConnectionRequest();
    if (!kPending) {
        return;
    }

    const ui::ConnectionOutcome kOutcome = ui::RequestConnection(
        graph_, kPending->first, kPending->second,
        [this](core::DataType type) { return TypeName(type); });
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
                         }});
}

}  // namespace editor