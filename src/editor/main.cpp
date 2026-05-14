#include <vector>

#include "graph.hpp"

int main() {
    InitWindow(800, 600, "Nebula Test");
    Vector2 cursor_postion = {0, 0};

    // Create graph
    core::Graph graph;

    // Create a visual node
    auto node1 = graph.AddNode(core::NodeBase::NodeKind::kLiteral, {500, 100});
    auto node2 =
        graph.AddNode(core::NodeBase::NodeKind::kVariable, {500, 200});
    auto node3 =
        graph.AddNode(core::NodeBase::NodeKind::kFunction, {200, 200});
    auto node4 =
        graph.AddNode(core::NodeBase::NodeKind::kFunctionInput, {200, 100});
    auto node5 =
        graph.AddNode(core::NodeBase::NodeKind::kFunctionOutput, {200, 300});
    auto node6 =
        graph.AddNode(core::NodeBase::NodeKind::kOperator, {500, 300});

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_H)) {
            if (IsCursorHidden()) {
                ShowCursor();
            }
            else {
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

        graph.DeleteWithMouse();
        graph.CheckNodeMovement();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw visual nodes in the graph
        graph.SelectWithMouse();
        graph.Draw();

        graph.LinkingWithMouse();
        graph.HandleContextMenu();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
