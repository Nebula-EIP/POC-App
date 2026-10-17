#include "raylib_wrapper.hpp"

namespace utils {

static_assert(static_cast<int>(WrappedKey::kEscape) == KEY_ESCAPE);
static_assert(static_cast<int>(WrappedKey::kBackspace) == KEY_BACKSPACE);
static_assert(static_cast<int>(WrappedKey::kLeftControl) == KEY_LEFT_CONTROL);
static_assert(static_cast<int>(WrappedKey::kRightControl) == KEY_RIGHT_CONTROL);
static_assert(static_cast<int>(WrappedKey::kDelete) == KEY_DELETE);
static_assert(static_cast<int>(WrappedKey::kX) == KEY_X);
static_assert(static_cast<int>(WrappedKey::kC) == KEY_C);
static_assert(static_cast<int>(WrappedKey::kH) == KEY_H);
static_assert(static_cast<int>(WrappedKey::kV) == KEY_V);
static_assert(static_cast<int>(WrappedKey::kRightShift) == KEY_RIGHT_SHIFT);
static_assert(static_cast<int>(WrappedKey::kRightAlt) == KEY_RIGHT_ALT);
static_assert(static_cast<int>(WrappedKey::kD) == KEY_D);
static_assert(static_cast<int>(WrappedKey::kH) == KEY_H);
static_assert(static_cast<int>(WrappedKey::kA) == KEY_A);
static_assert(static_cast<int>(WrappedKey::kQ) == KEY_Q);
static_assert(static_cast<int>(WrappedKey::kW) == KEY_W);
static_assert(static_cast<int>(WrappedKey::kZ) == KEY_Z);
static_assert(static_cast<int>(WrappedKey::kLeftShift) == KEY_LEFT_SHIFT);
static_assert(static_cast<int>(WrappedKey::kLeftAlt) == KEY_LEFT_ALT);

Font font = {};
bool font_loaded = false;

// Window functions

void InitRaylib(int width, int height, const char *title, unsigned int flags) {
    SetConfigFlags(flags);
    InitWindow(width, height, title);
    LoadDefaultFontWrapped("src/utils/Fonts/InterVariable.ttf", 64);
}

bool IsRaylibReady() { return IsWindowReady(); }

bool ShouldCloseRaylib() { return WindowShouldClose(); }

void SetFPS(int fps) { SetTargetFPS(fps); }

void CloseRaylib() {
    if (font_loaded) {
        UnloadFont(font);
        font_loaded = false;
    }
    CloseWindow();
}

void LoadDefaultFontWrapped(const char *path, int baseSize) {
    int codepoints[95 + 32];

    for (int i = 0; i < 95; i++) {
        codepoints[i] = 32 + i;
    }

    int french_codepoints[] = {
        192,  // À
        194,  // Â
        199,  // Ç
        200,  // È
        201,  // É
        202,  // Ê
        203,  // Ë
        206,  // Î
        207,  // Ï
        212,  // Ô
        217,  // Ù
        219,  // Û
        220,  // Ü
        224,  // à
        226,  // â
        231,  // ç
        232,  // è
        233,  // é
        234,  // ê
        235,  // ë
        238,  // î
        239,  // ï
        244,  // ô
        249,  // ù
        251,  // û
        252,  // ü
        338,  // Œ
        339,  // œ
        376,  // Ÿ
        255,  // ÿ
        171,  // «
        187   // »
    };

    int french_count = sizeof(french_codepoints) / sizeof(french_codepoints[0]);

    for (int i = 0; i < french_count; i++) {
        codepoints[95 + i] = french_codepoints[i];
    }

    int codepoint_count = 95 + french_count;

    if (font_loaded) {
        UnloadFont(font);
        font_loaded = false;
    }

    if (IsFileExtension(path, ".ttf") || IsFileExtension(path, ".otf")) {
        font = LoadFontEx(path, baseSize, codepoints, codepoint_count);
    } else {
        font = LoadFont(path);
    }

    if (font.texture.id != 0) {
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
        font_loaded = true;
    }
}

Font GetDefaultFontWrapped() { return font_loaded ? font : GetFontDefault(); }

// Frame functions

void BeginFrame() { BeginDrawing(); }

void EndFrame() { EndDrawing(); }

void ClearScreen() { ClearBackground(RAYWHITE); }

// Window queries

float GetScreenWidthWrapped() { return static_cast<float>(GetScreenWidth()); }

// Time

double GetTimeWrapped() { return GetTime(); }

// Cursor functions

WrappedVector2 GetCursorPositionWrapped() {
    Vector2 pos = GetMousePosition();
    WrappedVector2 wrapped_pos = {pos.x, pos.y};

    return wrapped_pos;
}

bool IsCursorHiddenWrapped() { return IsCursorHidden(); }

void ShowCursorWrapped() { ShowCursor(); }

void HideCursorWrapped() { HideCursor(); }

// Draw functions

void DrawRectangleWrapped(float x, float y, float width, float height,
                          WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    DrawRectangle(x, y, width, height, raylib_color);
}

void DrawRectangleRecWrapped(WrappedRectangle rectangle, WrappedColor color) {
    DrawRectangleRec(Rectangle{rectangle.x_, rectangle.y_, rectangle.width_,
                               rectangle.height_},
                     Color{color.r_, color.g_, color.b_, color.a_});
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

void DrawTextExWrapped(const char *text, float x, float y, int fontSize,
                       float spacing, WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    Vector2 position = {x, y};
    DrawTextEx(GetDefaultFontWrapped(), text, position, fontSize, spacing,
               raylib_color);
}

float MeasureTextWrapped(const char *text, int fontSize) {
    return static_cast<float>(MeasureText(text, fontSize));
}

// Input functions

bool IsRightClicked() { return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT); }

bool IsLeftClicked() { return IsMouseButtonPressed(MOUSE_BUTTON_LEFT); }

bool IsRightDown() { return IsMouseButtonDown(MOUSE_BUTTON_RIGHT); }

bool IsLeftDown() { return IsMouseButtonDown(MOUSE_BUTTON_LEFT); }

bool IsMiddleClicked() { return IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE); }

bool IsMiddleDown() { return IsMouseButtonDown(MOUSE_BUTTON_MIDDLE); }

bool IsKeyPressedWrapped(WrappedKey key) {
    return IsKeyPressed(static_cast<int>(key));
}

bool IsKeyDownWrapped(WrappedKey key) {
    return IsKeyDown(static_cast<int>(key));
}

int GetCharPressedWrapped() { return GetCharPressed(); }

float GetMouseWheelMoveWrapped() { return GetMouseWheelMove(); }

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