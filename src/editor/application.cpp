/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 29-09-2026
 */

#include "application.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>

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
constexpr utils::WrappedColor kConnectionHovered = {255, 255, 255, 255};
constexpr utils::WrappedColor kConnectionGhost = {150, 150, 150, 128};

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
    // Items without a command are not implemented yet: they are greyed out.
    top_bar_.AddMenu("Fichier")
        .AddAction("Nouveau")
        .AddAction("Ouvrir...")
        .AddAction("Enregistrer")
        .AddSeparator()
        .AddAction("Quitter", [this] { should_quit_ = true; });

    top_bar_.AddMenu("Affichage")
        .AddAction("Recentrer", [this] { RecenterView(); })
        .AddAction("Zoom 100 %", [this] { ResetZoom(); });
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
    // The top bar goes first: when it owns the mouse (click on the bar, open
    // menu...), the canvas must not see it. Its commands run inside Update().
    const bool kPointerCaptured = top_bar_.Update(ui::TopBar::ReadInput());
    renderer_.ProcessInput(camera_, kPointerCaptured);

    cursor_position_ = utils::GetCursorPositionWrapped();
    const bool kCanvasClicked = !kPointerCaptured && utils::IsLeftClicked();

    if (kCanvasClicked) {
        for (const auto &[id, view] : renderer_.node_canvas_.Views()) {
            view->ClearErrorPin();
        }
    }

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
                    view->second->SetErrorPin(
                        end_hit.pin_id_,
                        end_hit.part_ == ui::HitPart::kInputPin);
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
                    view->second->SetErrorPin(
                        start_hit.pin_id_,
                        start_hit.part_ == ui::HitPart::kInputPin);
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

    if (hovered_connection_ &&
        utils::IsKeyPressedWrapped(utils::WrappedKey::kDelete)) {
        graph_.Disconnect(*hovered_connection_);
        hovered_connection_ = std::nullopt;
    }

    if (utils::IsKeyPressedWrapped(utils::WrappedKey::kH)) {
        if (utils::IsCursorHiddenWrapped()) {
            utils::ShowCursorWrapped();
        } else {
            utils::HideCursorWrapped();
        }
    }
    if (utils::IsKeyDownWrapped(utils::WrappedKey::kLeftControl) &&
        utils::IsKeyPressedWrapped(utils::WrappedKey::kD)) {
        // Duplicate the selected node
    }
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
            const bool kIsHovered =
                hovered_connection_ && *hovered_connection_ == id;
            const auto kColor =
                kIsHovered ? kConnectionHovered : kConnectionNormal;
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
                     .draw_ui_ = [this]() { top_bar_.Draw(); }});
}

}  // namespace editor