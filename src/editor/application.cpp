/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 26-09-2026
 */

#include "application.hpp"

#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>

#include "utils/logger.hpp"

namespace editor {

namespace {
constexpr int kDefaultWidth = 800;
constexpr int kDefaultHeight = 600;
constexpr int kTargetFps = 60;
constexpr float kNodeHalfWidth = 50.0F;
constexpr float kNodeHalfHeight = 25.0F;
constexpr float kLoadingTextPosition = 20.0F;
constexpr int kLoadingTextSize = 20;
}  // namespace

Application::Application(std::filesystem::path module_path)
    : module_path_(std::move(module_path)) {
    utils::InitRaylib(kDefaultWidth, kDefaultHeight, "Nebula", true);

    if (!utils::IsRaylibReady()) {
        throw std::runtime_error("Raylib window initialization failed");
    }
    window_ready_ = true;
    utils::SetFPS(kTargetFps);
}

Application::~Application() {
    // Explicit for clarity, though member declaration order already
    // guarantees it: drop the UI and the graph's nodes before loader_'s
    // destructor unloads the module and closes its shared library.
    module_ = nullptr;

    if (window_ready_ && utils::IsRaylibReady()) {
        utils::CloseRaylib();
    }
}

int Application::Run() {
    // The window is up: give the user something on screen before the
    // (potentially slow) module load.
    DrawInitialFrame();

    LoadModule();
    BuildMenus();

    while (!utils::ShouldCloseRaylib() && !should_quit_) {
        ProcessInput();
        DrawFrame();
    }

    return 0;
}

void Application::DrawInitialFrame() {
    utils::BeginFrame();
    utils::ClearScreen();
    utils::DrawTextWrapped("Loading module...", kLoadingTextPosition,
                           kLoadingTextPosition, kLoadingTextSize,
                           utils::kDarkgray);
    utils::EndFrame();
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
    // Function to build the top bar menus
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
    node_canvas_.AddNode(node, std::move(title), position);
}

utils::WrappedVector2 Application::SpawnPosition() const {
    return utils::WrappedVector2{cursor_position_.x_ - kNodeHalfWidth,
                                 cursor_position_.y_ - kNodeHalfHeight};
}

void Application::ProcessInput() {
    cursor_position_ = utils::GetCursorPositionWrapped();
    node_canvas_.ProcessInput();

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
    utils::BeginFrame();
    utils::ClearScreen();

    node_canvas_.Draw();

    utils::EndFrame();
}

}  // namespace editor