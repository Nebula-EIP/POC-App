#include "raylib_wrapper.hpp"

namespace utils {
void InitRaylib(int width, int height, const char *title) {
    InitWindow(width, height, title);
}

void SetFPS(int fps) { SetTargetFPS(fps); }

void CloseRaylib() { CloseWindow(); }

void ClearScreen() { ClearBackground(RAYWHITE); }

// Cursor functions

WrappedVector2 GetCursorPositionWrapped() {
    Vector2 pos = GetMousePosition();
    WrappedVector2 wrapped_pos = {pos.x, pos.y};

    return wrapped_pos;
}

static std::unordered_map<CursorType, Texture2D> g_cursor_textures;
static CursorType g_current_cursor = CursorType::Default;

static const char *CursorPath(CursorType type) {
    switch (type) {
        case CursorType::Click:
            return "assets/cursor/click.png";

        case CursorType::Grab:
            return "assets/cursor/grab.png";

        case CursorType::Hand:
            return "assets/cursor/hand.png";

        case CursorType::Input:
            return "assets/cursor/input.png";

        default:
            return "assets/cursor/default.png";
    }
}

void InitCursorSystem() {
    g_cursor_textures[CursorType::Default] =
        LoadTexture(CursorPath(CursorType::Default));
    g_cursor_textures[CursorType::Click] =
        LoadTexture(CursorPath(CursorType::Click));
    g_cursor_textures[CursorType::Grab] =
        LoadTexture(CursorPath(CursorType::Grab));
    g_cursor_textures[CursorType::Hand] =
        LoadTexture(CursorPath(CursorType::Hand));
    g_cursor_textures[CursorType::Input] =
        LoadTexture(CursorPath(CursorType::Input));

    HideCursor();
}

void ShutdownCursorSystem() {
    for (auto &tex : g_cursor_textures) UnloadTexture(tex.second);

    g_cursor_textures.clear();

    ShowCursor();
}

void RequestCursor(CursorType cursor) { g_current_cursor = cursor; }

CursorType GetCursor() { return g_current_cursor; }

void DrawCursor() {
    auto it = g_cursor_textures.find(g_current_cursor);

    if (it == g_cursor_textures.end()) return;

    Vector2 mouse = GetMousePosition();

    Texture2D tex = it->second;

    DrawTexture(tex, static_cast<int>(mouse.x - tex.width / 2),
                static_cast<int>(mouse.y), WHITE);
}

void UpdateCursor() {
    using CT = utils::CursorType;

    if (utils::IsTextInputFocused()) {
        utils::RequestCursor(CT::Input);
        return;
    }

    if (utils::IsDragging()) {
        utils::RequestCursor(CT::Grab);
        return;
    }

    if (utils::IsHoveringUI()) {
        utils::RequestCursor(CT::Hand);
        return;
    }

    if (utils::IsHoveringGraphNode()) {
        utils::RequestCursor(CT::Click);
        return;
    }

    utils::RequestCursor(CT::Default);
}

void SetHoveringUI(bool v) { gHoveringUI = v; }

void SetDragging(bool v) { gDragging = v; }

void SetHoveringGraphNode(bool v) { gHoveringGraphNode = v; }

void SetTextInputFocused(bool v) { gTextInputFocused = v; }

bool IsHoveringUI() { return gHoveringUI; }

bool IsDragging() { return gDragging; }

bool IsHoveringGraphNode() { return gHoveringGraphNode; }

bool IsTextInputFocused() { return gTextInputFocused; }

// Draw functions

void DrawRectangleWrapped(float x, float y, float width, float height,
                          WrappedColor color) {
    DrawRectangle(x, y, width, height, color);
}

void DrawRectangleLinesWrapped(float x, float y, float width, float height,
                               WrappedColor color) {
    DrawRectangleLines(x, y, width, height, color);
}

void DrawCircleWrapped(float centerX, float centerY, float radius,
                       WrappedColor color) {
    DrawCircle(centerX, centerY, radius, color);
}

void DrawLineWrapped(WrappedVector2 start, WrappedVector2 end, float thick,
                     WrappedColor color) {
    Vector2 start_vec = {start.x, start.y};
    Vector2 end_vec = {end.x, end.y};

    DrawLineEx(start_vec, end_vec, thick, color);
}

void DrawLineBezierWrapped(WrappedVector2 start, WrappedVector2 end,
                           float thick, WrappedColor color) {
    Vector2 start_vec = {start.x, start.y};
    Vector2 end_vec = {end.x, end.y};

    DrawLineBezier(start_vec, end_vec, thick, color);
}

void DrawTextWrapped(const char *text, float x, float y, int fontSize,
                     WrappedColor color) {
    DrawText(text, x, y, fontSize, color);
}

// Input functions

bool isRightClicked() { return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT); }

bool isLeftClicked() { return IsMouseButtonPressed(MOUSE_BUTTON_LEFT); }

bool isRightDown() { return IsMouseButtonDown(MOUSE_BUTTON_RIGHT); }

bool isLeftDown() { return IsMouseButtonDown(MOUSE_BUTTON_LEFT); }

float GetMouseWheelMoveWrapped() { return GetMouseWheelMove(); }

bool IsKeyDownWrapped(int key) { return IsKeyDown(key); }

bool IsKeyPressedWrapped(int key) { return IsKeyPressed(key); }

bool isMiddleClicked() { return IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE); }

bool isMiddleDown() { return IsMouseButtonDown(MOUSE_BUTTON_MIDDLE); }

// Collision functions

bool CheckCollisionRecsWrapped(WrappedRectangle rec1, WrappedRectangle rec2) {
    Rectangle rec1_ray = {rec1.x, rec1.y, rec1.width, rec1.height};
    Rectangle rec2_ray = {rec2.x, rec2.y, rec2.width, rec2.height};

    return CheckCollisionRecs(rec1_ray, rec2_ray);
}

bool CheckCollisionCirclesWrapped(WrappedCircle circle1,
                                  WrappedCircle circle2) {
    Vector2 center1_vec = {circle1.centerX, circle1.centerY};
    Vector2 center2_vec = {circle2.centerX, circle2.centerY};

    return CheckCollisionCircles(center1_vec, circle1.radius, center2_vec,
                                 circle2.radius);
}

bool CheckCollisionCircleRecWrapped(WrappedCircle circle,
                                    WrappedRectangle rec) {
    Vector2 center_vec = {circle.centerX, circle.centerY};
    Rectangle rec_ray = {rec.x, rec.y, rec.width, rec.height};

    return CheckCollisionCircleRec(center_vec, circle.radius, rec_ray);
}

bool CheckCollisionCircleLineWrapped(WrappedCircle circle, WrappedVector2 start,
                                     WrappedVector2 end) {
    Vector2 center_vec = {circle.centerX, circle.centerY};
    Vector2 start_vec = {start.x, start.y};
    Vector2 end_vec = {end.x, end.y};

    return CheckCollisionCircleLine(center_vec, circle.radius, start_vec,
                                    end_vec);
}

bool CheckCollisionPointRecWrapped(WrappedVector2 point, WrappedRectangle rec) {
    Vector2 point_vec = {point.x, point.y};
    Rectangle rec_ray = {rec.x, rec.y, rec.width, rec.height};

    return CheckCollisionPointRec(point_vec, rec_ray);
}

bool CheckCollisionPointCircleWrapped(WrappedVector2 point,
                                      WrappedCircle circle) {
    Vector2 point_vec = {point.x, point.y};
    Vector2 center_vec = {circle.centerX, circle.centerY};

    return CheckCollisionPointCircle(point_vec, center_vec, circle.radius);
}

bool CheckCollisionPointTriangleWrapped(WrappedVector2 point, WrappedVector2 v1,
                                        WrappedVector2 v2, WrappedVector2 v3) {
    Vector2 point_vec = {point.x, point.y};
    Vector2 v1_vec = {v1.x, v1.y};
    Vector2 v2_vec = {v2.x, v2.y};
    Vector2 v3_vec = {v3.x, v3.y};

    return CheckCollisionPointTriangle(point_vec, v1_vec, v2_vec, v3_vec);
}

bool CheckCollisionPointLineWrapped(WrappedVector2 point, WrappedVector2 start,
                                    WrappedVector2 end, int threshold) {
    Vector2 point_vec = {point.x, point.y};
    Vector2 start_vec = {start.x, start.y};
    Vector2 end_vec = {end.x, end.y};

    return CheckCollisionPointLine(point_vec, start_vec, end_vec, threshold);
}

bool CheckCollisionPointPolyWrapped(WrappedVector2 point,
                                    WrappedVector2 *vertices, int vertexCount) {
    Vector2 point_vec = {point.x, point.y};
    Vector2 *vertices_vec = new Vector2[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
        vertices_vec[i] = {vertices[i].x, vertices[i].y};
    }

    bool result = CheckCollisionPointPoly(point_vec, vertices_vec, vertexCount);
    delete[] vertices_vec;
    return result;
}

bool CheckCollisionLinesWrapped(WrappedVector2 start1, WrappedVector2 end1,
                                WrappedVector2 start2, WrappedVector2 end2) {
    Vector2 start1_vec = {start1.x, start1.y};
    Vector2 end1_vec = {end1.x, end1.y};
    Vector2 start2_vec = {start2.x, start2.y};
    Vector2 end2_vec = {end2.x, end2.y};
    Vector2 collision_point_vec;

    return CheckCollisionLines(start1_vec, end1_vec, start2_vec, end2_vec,
                               &collision_point_vec);
}

WrappedRectangle GetCollisionRecWrapped(WrappedRectangle rec1,
                                        WrappedRectangle rec2) {
    Rectangle rec1_ray = {rec1.x, rec1.y, rec1.width, rec1.height};
    Rectangle rec2_ray = {rec2.x, rec2.y, rec2.width, rec2.height};

    Rectangle collision_rec = GetCollisionRec(rec1_ray, rec2_ray);
    return {collision_rec.x, collision_rec.y, collision_rec.width,
            collision_rec.height};
}
}  // namespace utils