#include "graph.hpp"
#include <vector>

int main()
{
    InitWindow(800, 600, "Nebula Test");
    Vector2 cursorPostion = { 0, 0 };

    //Create graph
    core::Graph graph_;

    //Create a visual node
    auto node1 = graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {500, 100});
    auto node2 = graph_.AddNode(core::NodeBase::NodeKind::kVariable, {500, 200});
    auto node3 = graph_.AddNode(core::NodeBase::NodeKind::kFunction, {200, 200});
    auto node4 = graph_.AddNode(core::NodeBase::NodeKind::kFunctionInput, {200, 100});
    auto node5 = graph_.AddNode(core::NodeBase::NodeKind::kFunctionOutput, {200, 300});
    auto node6 = graph_.AddNode(core::NodeBase::NodeKind::kOperator, {500, 300});

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_H)) {
            if (IsCursorHidden()) ShowCursor();
            else HideCursor();
        }

        if (IsKeyPressed(KEY_Q)) {
            graph_.AddNode(core::NodeBase::NodeKind::kVariable, {cursorPostion.x - 50, cursorPostion.y - 25});
        }
        if (IsKeyPressed(KEY_W)) {
            graph_.AddNode(core::NodeBase::NodeKind::kLiteral, {cursorPostion.x - 50, cursorPostion.y - 25});
        }

        cursorPostion = GetMousePosition();

        graph_.DeleteWithMouse();
        graph_.CheckNodeMovement();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw visual nodes in the graph
        graph_.SelectWithMouse();
        graph_.Draw();

        graph_.LinkingWithMouse();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
