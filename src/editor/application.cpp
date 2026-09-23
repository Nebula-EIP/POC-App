/**
 * @file application.cpp
 * @brief Implementation of the Application class, which manages the main loop,
 * window, and module loading for the editor.
 *
 * @author Created by NathanBezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by NathanBezard
 * @date Last modified on 21-09-2026
 */

#include "application.hpp"

#include <stdexcept>
#include <utility>

namespace editor {

namespace {
constexpr int kDefaultWidth = 800;
constexpr int kDefaultHeight = 600;
constexpr int kTargetFps = 60;
constexpr float kNodeHalfWidth = 50.0F;
constexpr float kNodeHalfHeight = 25.0F;
}  // namespace

Application::Application(std::filesystem::path module_path)
    : module_path_(std::move(module_path)) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(kDefaultWidth, kDefaultHeight, "Nebula");

    if (!IsWindowReady()) {
        throw std::runtime_error("Raylib window initialization failed");
    }
    window_ready_ = true;
    SetTargetFPS(kTargetFps);
}

Application::~Application() {
    // Explicit for clarity, though member declaration order already
    // guarantees it: drop the UI and the graph's nodes before loader_'s
    // destructor unloads the module and closes its shared library.
    module_ = nullptr;

    if (window_ready_ && IsWindowReady()) {
        CloseWindow();
    }
}

int Application::Run() {
    // The window is up: give the user something on screen before the
    // (potentially slow) module load.
    DrawInitialFrame();

    LoadModule();
    BuildMenus();

    while (!WindowShouldClose() && !should_quit_) {
        ProcessInput();
        DrawFrame();
    }

    return 0;
}

void Application::DrawInitialFrame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Loading module...", 20, 20, 20, DARKGRAY);
    EndDrawing();
}

void Application::LoadModule() {
    module_id_ = loader_.Load(module_path_);
    module_ = loader_.Module(module_id_);

    if (module_ == nullptr) {
        throw std::runtime_error("Loaded module " + module_path_.string() +
                                 " could not be resolved by id");
    }

    TraceLog(LOG_INFO, "Loaded module %s", module_path_.string().c_str());
}

void Application::BuildMenus() {
    // Function to build the top bar menus
}

void Application::CreateNodeFromConfiguration(
    core::NodeType type, const core::capa::NodeConfiguration &config,
    Vector2 position) {
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

Vector2 Application::SpawnPosition() const {
    return Vector2{cursor_position_.x - kNodeHalfWidth,
                   cursor_position_.y - kNodeHalfHeight};
}

void Application::ProcessInput() {
    cursor_position_ = GetMousePosition();

    if (IsKeyPressed(KEY_H)) {
        if (IsCursorHidden()) {
            ShowCursor();
        } else {
            HideCursor();
        }
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
        // Duplicate the selected node
    }

    // Update top bar
}

void Application::DrawFrame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw graph an top bar

    EndDrawing();
}

}  // namespace editor