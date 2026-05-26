#include <vector>

#include "graph.hpp"
#include "ui/top_bar.hpp"

int main() {
    InitWindow(800, 600, "Nebula Test");
    Vector2 cursor_postion = {0, 0};

    // Create graph
    core::Graph graph;

    std::vector<editor_ui::Menu> menus;

    menus.push_back(editor_ui::Menu{
        "File",
        {
            editor_ui::MenuItem{"New", [&graph]() { graph = core::Graph{}; }},
            editor_ui::MenuItem{"Save", []() {
                TraceLog(LOG_INFO, "Save requested from top bar");
            }},
            editor_ui::MenuItem{"Load", []() {
                TraceLog(LOG_INFO, "Load requested from top bar");
            }},
            editor_ui::MenuItem{"Quit", []() { CloseWindow(); }},
        },
    });

    menus.push_back(editor_ui::Menu{
        "Add",
        {
            editor_ui::MenuItem{"Variable", [&graph, &cursor_postion]() {
                graph.AddNode(core::NodeBase::NodeKind::kVariable,
                              {cursor_postion.x - 50, cursor_postion.y - 25});
            }},
            editor_ui::MenuItem{"Literal", [&graph, &cursor_postion]() {
                graph.AddNode(core::NodeBase::NodeKind::kLiteral,
                              {cursor_postion.x - 50, cursor_postion.y - 25});
            }},
        },
    });

    menus.push_back(editor_ui::Menu{
        "Edit",
        {
            editor_ui::MenuItem{"Duplicate selection", [&graph]() {
                graph.DuplicateSelectedNode();
            }},
            editor_ui::MenuItem{"Delete under cursor", [&graph]() {
                graph.DeleteWithMouse();
            }},
        },
    });

    editor_ui::TopBar top_bar(std::move(menus));

    // Create a visual node
    auto node1 = graph.AddNode(core::NodeBase::NodeKind::kLiteral, {500, 100});
    auto node2 = graph.AddNode(core::NodeBase::NodeKind::kVariable, {500, 200});
    auto node3 = graph.AddNode(core::NodeBase::NodeKind::kFunction, {200, 200});
    auto node4 =
        graph.AddNode(core::NodeBase::NodeKind::kFunctionInput, {200, 100});
    auto node5 =
        graph.AddNode(core::NodeBase::NodeKind::kFunctionOutput, {200, 300});
    auto node6 = graph.AddNode(core::NodeBase::NodeKind::kOperator, {500, 300});

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_H)) {
            if (IsCursorHidden()) {
                ShowCursor();
            } else {
                HideCursor();
            }
        }

        if (IsKeyPressed(KEY_Q)) {
            graph.AddNode(core::NodeBase::NodeKind::kVariable,
                          {cursor_postion.x - 50, cursor_postion.y - 25});
        }
        if (IsKeyPressed(KEY_W)) {
            graph.AddNode(core::NodeBase::NodeKind::kLiteral,
                          {cursor_postion.x - 50, cursor_postion.y - 25});
        }
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D)) {
            graph.DuplicateSelectedNode();
        }

        cursor_postion = GetMousePosition();
        top_bar.Update();

        if (!top_bar.BlocksGraphInput()) {
            graph.DeleteWithMouse();
            graph.CheckNodeMovement();
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw visual nodes in the graph
        if (!top_bar.BlocksGraphInput()) {
            graph.SelectWithMouse();
        }
        graph.Draw();

        if (!top_bar.BlocksGraphInput()) {
            graph.LinkingWithMouse();
            graph.HandleContextMenu();
        }

        top_bar.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
