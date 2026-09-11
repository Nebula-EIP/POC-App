#include "raylib_wrapper.hpp"

namespace utils {
void InitRaylib(int width, int height, const char *title) {
    InitWindow(width, height, title);
}

void SetFPS(int fps) { SetTargetFPS(fps); }

void CloseRaylib() { CloseWindow(); }

void ClearScreen() { ClearBackground(RAYWHITE); }

WrappedVector2 GetCursorPositionWrapped() {
    Vector2 pos = GetMousePosition();
    WrappedVector2 wrapped_pos = {pos.x, pos.y};

    return wrapped_pos;
}

// Draw functions

void DrawRectangleWrapped(float x, float y, float width, float height,
                          WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    DrawRectangle(x, y, width, height, raylib_color);
}

void DrawRectangleLinesWrapped(float x, float y, float width, float height,
                               WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    DrawRectangleLines(x, y, width, height, raylib_color);
}

void DrawCircleWrapped(float centerX, float centerY, float radius,
                       WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    DrawCircle(centerX, centerY, radius, raylib_color);
}

void DrawLineWrapped(WrappedVector2 start, WrappedVector2 end, float thick,
                     WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    Vector2 start_vec = {start.x_, start.y_};
    Vector2 end_vec = {end.x_, end.y_};

    DrawLineEx(start_vec, end_vec, thick, raylib_color);
}

void DrawLineBezierWrapped(WrappedVector2 start, WrappedVector2 end,
                           float thick, WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    Vector2 start_vec = {start.x_, start.y_};
    Vector2 end_vec = {end.x_, end.y_};

    DrawLineBezier(start_vec, end_vec, thick, raylib_color);
}

void DrawTextWrapped(const char *text, float x, float y, int fontSize,
                     WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    DrawText(text, x, y, fontSize, raylib_color);
}

// Input functions

bool IsRightClicked() {
    return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}

bool IsLeftClicked() {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool IsRightDown() {
    return IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
}

bool IsLeftDown() {
    return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

// Collision functions

bool CheckCollisionRecsWrapped(WrappedRectangle rec1, WrappedRectangle rec2) {
    Rectangle rec1_ray = {rec1.x_, rec1.y_, rec1.width_, rec1.height_};
    Rectangle rec2_ray = {rec2.x_, rec2.y_, rec2.width_, rec2.height_};

    return CheckCollisionRecs(rec1_ray, rec2_ray);
}

bool CheckCollisionCirclesWrapped(WrappedCircle circle1,
                                  WrappedCircle circle2) {
    Vector2 center1_vec = {circle1.center_x_, circle1.center_y_};
    Vector2 center2_vec = {circle2.center_x_, circle2.center_y_};

    return CheckCollisionCircles(center1_vec, circle1.radius_, center2_vec,
                                 circle2.radius_);
}

bool CheckCollisionCircleRecWrapped(WrappedCircle circle,
                                    WrappedRectangle rec) {
    Vector2 center_vec = {circle.center_x_, circle.center_y_};
    Rectangle rec_ray = {rec.x_, rec.y_, rec.width_, rec.height_};

    return CheckCollisionCircleRec(center_vec, circle.radius_, rec_ray);
}

bool CheckCollisionCircleLineWrapped(WrappedCircle circle, WrappedVector2 start,
                                     WrappedVector2 end) {
    Vector2 center_vec = {circle.center_x_, circle.center_y_};
    Vector2 start_vec = {start.x_, start.y_};
    Vector2 end_vec = {end.x_, end.y_};

    return CheckCollisionCircleLine(center_vec, circle.radius_, start_vec,
                                    end_vec);
}

bool CheckCollisionPointRecWrapped(WrappedVector2 point, WrappedRectangle rec) {
    Vector2 point_vec = {point.x_, point.y_};
    Rectangle rec_ray = {rec.x_, rec.y_, rec.width_, rec.height_};

    return CheckCollisionPointRec(point_vec, rec_ray);
}

bool CheckCollisionPointCircleWrapped(WrappedVector2 point,
                                      WrappedCircle circle) {
    Vector2 point_vec = {point.x_, point.y_};
    Vector2 center_vec = {circle.center_x_, circle.center_y_};

    return CheckCollisionPointCircle(point_vec, center_vec, circle.radius_);
}

bool CheckCollisionPointTriangleWrapped(WrappedVector2 point, WrappedVector2 v1,
                                        WrappedVector2 v2, WrappedVector2 v3) {
    Vector2 point_vec = {point.x_, point.y_};
    Vector2 v1_vec = {v1.x_, v1.y_};
    Vector2 v2_vec = {v2.x_, v2.y_};
    Vector2 v3_vec = {v3.x_, v3.y_};

    return CheckCollisionPointTriangle(point_vec, v1_vec, v2_vec, v3_vec);
}

bool CheckCollisionPointLineWrapped(WrappedVector2 point, WrappedVector2 start,
                                    WrappedVector2 end, int threshold) {
    Vector2 point_vec = {point.x_, point.y_};
    Vector2 start_vec = {start.x_, start.y_};
    Vector2 end_vec = {end.x_, end.y_};

    return CheckCollisionPointLine(point_vec, start_vec, end_vec, threshold);
}

bool CheckCollisionPointPolyWrapped(WrappedVector2 point,
                                    WrappedVector2 *vertices, int vertexCount) {
    Vector2 point_vec = {point.x_, point.y_};
    Vector2 *vertices_vec = new Vector2[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
        vertices_vec[i] = {vertices[i].x_, vertices[i].y_};
    }

    bool result = CheckCollisionPointPoly(point_vec, vertices_vec, vertexCount);
    delete[] vertices_vec;
    return result;
}

bool CheckCollisionLinesWrapped(WrappedVector2 start1, WrappedVector2 end1,
                                WrappedVector2 start2, WrappedVector2 end2) {
    Vector2 start1_vec = {start1.x_, start1.y_};
    Vector2 end1_vec = {end1.x_, end1.y_};
    Vector2 start2_vec = {start2.x_, start2.y_};
    Vector2 end2_vec = {end2.x_, end2.y_};
    Vector2 collision_point_vec;

    return CheckCollisionLines(start1_vec, end1_vec, start2_vec, end2_vec,
                               &collision_point_vec);
}

WrappedRectangle GetCollisionRecWrapped(WrappedRectangle rec1,
                                        WrappedRectangle rec2) {
    Rectangle rec1_ray = {rec1.x_, rec1.y_, rec1.width_, rec1.height_};
    Rectangle rec2_ray = {rec2.x_, rec2.y_, rec2.width_, rec2.height_};

    Rectangle collision_rec = GetCollisionRec(rec1_ray, rec2_ray);
    return {collision_rec.x, collision_rec.y, collision_rec.width,
            collision_rec.height};
}
}  // namespace utils