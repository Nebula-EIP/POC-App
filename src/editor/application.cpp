/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 26-09-2026
 */

#include "application.hpp"
 
#include <source_location>
#include <stdexcept>
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
}  // namespace
 
Application::Application(std::filesystem::path module_path)
    // renderer_'s constructor opens the window and throws on failure — this
    // is what used to be utils::InitRaylib()/IsRaylibReady()/window_ready_
    // here; Renderer's own RAII destructor replaces the manual
    // CloseRaylib() check in ~Application, so ~Application is now = default
    // and window_ready_ is gone entirely.
    : module_path_(std::move(module_path)),
     renderer_(kDefaultWidth, kDefaultHeight, "Nebula") {}
 
int Application::Run() {
    // The window is up: give the user something on screen before the
    // (potentially slow) module load.
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
    // Renderer doesn't know the window size until asked; feed it to camera_
    // once so DrawGrid (harmlessly drawn here too) is centered correctly.
    const utils::WrappedVector2 kScreenSize = renderer_.ScreenSize();
    camera_.SetViewport(kScreenSize.x_, kScreenSize.y_);
 
    renderer_.RenderFrame(
        camera_, render::FrameCallbacks{
                    .draw_ui =
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
}
 
void Application::BuildMenus() {
    // Function to build the top bar menus
}
 
void Application::CreateNodeFromConfiguration(
    core::NodeType type, const core::capa::NodeConfiguration &config,
    utils::WrappedVector2 position) {
    (void)position;  // For when we need position for node display
 
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
}
 
utils::WrappedVector2 Application::SpawnPosition() const {
    return utils::WrappedVector2{cursor_position_.x_ - kNodeHalfWidth,
                                 cursor_position_.y_ - kNodeHalfHeight};
}
 
void Application::ProcessInput() {
    // Middle-click pan, wheel zoom (clamped 25-400%), and the viewport
    // refresh on resize all live in Camera/Renderer now — this used to be
    // absent here entirely.
    renderer_.ProcessInput(camera_);
 
    cursor_position_ = utils::GetCursorPositionWrapped();
 
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
 
    // Update top bar
}
 
void Application::DrawFrame() {
    // TODO: wire draw_links/draw_nodes to the graphical adapter over graph_,
    // and draw_ui to the top bar, once they exist. Layer order (grid, links,
    // nodes, UI) is enforced inside Renderer::RenderFrame — nothing to do
    // here for that.
    renderer_.RenderFrame(camera_, render::FrameCallbacks{});
}
 
}  // namespace editor
