#include "raylib_wrapper.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

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
Font bold_font = {};
bool bold_font_loaded = false;

// Window functions

void InitRaylib(int width, int height, const char *title, unsigned int flags) {
    SetConfigFlags(flags);
    InitWindow(width, height, title);
    LoadDefaultFontWrapped("src/utils/Fonts/InterVariable.ttf", 64);
    LoadBoldFontWrapped("src/utils/Fonts/Inter-Bold.ttf", 64);
}

bool IsRaylibReady() { return IsWindowReady(); }

bool ShouldCloseRaylib() { return WindowShouldClose(); }

void SetFPS(int fps) { SetTargetFPS(fps); }

void CloseRaylib() {
    if (font_loaded) {
        UnloadFont(font);
        font_loaded = false;
    }
    if (bold_font_loaded) {
        UnloadFont(bold_font);
        bold_font_loaded = false;
    }
    CloseWindow();
}

namespace {

/**
 * @brief Loads a font with the ASCII and French characters, smoothed when
 * scaled.
 *
 * @return The font, with a texture id of 0 when it could not be loaded.
 */
Font LoadFontWithCodepoints(const char *path, int baseSize) {
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

    Font loaded = {};
    if (IsFileExtension(path, ".ttf") || IsFileExtension(path, ".otf")) {
        loaded = LoadFontEx(path, baseSize, codepoints, codepoint_count);
    } else {
        loaded = LoadFont(path);
    }

    if (loaded.texture.id != 0) {
        SetTextureFilter(loaded.texture, TEXTURE_FILTER_BILINEAR);
    }
    return loaded;
}

}  // namespace

void LoadDefaultFontWrapped(const char *path, int baseSize) {
    if (font_loaded) {
        UnloadFont(font);
        font_loaded = false;
    }
    font = LoadFontWithCodepoints(path, baseSize);
    font_loaded = font.texture.id != 0;
}

void LoadBoldFontWrapped(const char *path, int baseSize) {
    if (bold_font_loaded) {
        UnloadFont(bold_font);
        bold_font_loaded = false;
    }
    bold_font = LoadFontWithCodepoints(path, baseSize);
    bold_font_loaded = bold_font.texture.id != 0;
}

Font GetDefaultFontWrapped() { return font_loaded ? font : GetFontDefault(); }

Font GetFontWrapped(FontWeight weight) {
    if (weight == FontWeight::kBold && bold_font_loaded) {
        return bold_font;
    }
    return GetDefaultFontWrapped();
}

// Frame functions

void BeginFrame() { BeginDrawing(); }

void EndFrame() { EndDrawing(); }

void ClearScreen() { ClearBackground(RAYWHITE); }

// Window queries

float GetScreenWidthWrapped() { return static_cast<float>(GetScreenWidth()); }

// Time

double GetTimeWrapped() { return GetTime(); }

bool IsKeyPressedRepeatWrapped(WrappedKey key) {
    return IsKeyPressedRepeat(static_cast<int>(key));
}

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

namespace {

// Segments per rounded corner and per half circle.
constexpr int kCornerSegments = 8;
constexpr int kArcSegments = 24;

/**
 * @brief Converts a corner radius into raylib's roundness, a ratio of the
 * shortest side of the rectangle.
 */
float Roundness(WrappedRectangle rectangle, float radius) {
    const float kShortest = std::min(rectangle.width_, rectangle.height_);
    if (kShortest <= 0.0F || radius <= 0.0F) {
        return 0.0F;
    }
    return std::min(2.0F * radius / kShortest, 1.0F);
}

}  // namespace

void DrawRectangleRoundedWrapped(WrappedRectangle rectangle, float radius,
                                 WrappedColor color) {
    DrawRectangleRounded(Rectangle{rectangle.x_, rectangle.y_, rectangle.width_,
                                   rectangle.height_},
                         Roundness(rectangle, radius), kCornerSegments,
                         Color{color.r_, color.g_, color.b_, color.a_});
}

void DrawRectangleRoundedLinesWrapped(WrappedRectangle rectangle, float radius,
                                      float thick, WrappedColor color) {
    DrawRectangleRoundedLinesEx(Rectangle{rectangle.x_, rectangle.y_,
                                          rectangle.width_, rectangle.height_},
                                Roundness(rectangle, radius), kCornerSegments,
                                thick,
                                Color{color.r_, color.g_, color.b_, color.a_});
}

void DrawShadowWrapped(WrappedRectangle rectangle, float radius,
                       WrappedVector2 offset, float blur, WrappedColor color) {
    constexpr int kMaxLayers = 12;
    if (blur <= 0.0F) {
        DrawRectangleRoundedWrapped(
            {rectangle.x_ + offset.x_, rectangle.y_ + offset.y_,
             rectangle.width_, rectangle.height_},
            radius, color);
        return;
    }

    // Stacked translucent layers, from slightly inside the shadow's edge to
    // `blur` outside of it: the opacity fades out across the blur, and the
    // layers add up to `color` where they all overlap.
    const int kLayers =
        std::clamp(static_cast<int>(std::lround(2.0F * blur)), 1, kMaxLayers);
    const float kAlpha = static_cast<float>(color.a_);
    WrappedColor layer_color = color;
    layer_color.a_ = static_cast<unsigned char>(std::max(
        1L, std::lround(255.0F * (1.0F - std::pow(1.0F - kAlpha / 255.0F,
                                                  1.0F / kLayers)))));
    for (int layer = 0; layer < kLayers; ++layer) {
        const float kSpread = -blur / 2.0F + 1.5F * blur *
                                                 static_cast<float>(layer + 1) /
                                                 static_cast<float>(kLayers);
        DrawRectangleRoundedWrapped({rectangle.x_ + offset.x_ - kSpread,
                                     rectangle.y_ + offset.y_ - kSpread,
                                     rectangle.width_ + 2.0F * kSpread,
                                     rectangle.height_ + 2.0F * kSpread},
                                    std::max(radius + kSpread, 0.0F),
                                    layer_color);
    }
}

void DrawRectangleGradientVWrapped(WrappedRectangle rectangle, WrappedColor top,
                                   WrappedColor bottom) {
    // The float variant keeps sub-pixel positions under a zoomed camera.
    // raylib 5.5 colors the corners top-left, bottom-left, bottom-right then
    // top-right, whatever its parameter names say (as DrawRectangleGradientV).
    DrawRectangleGradientEx(Rectangle{rectangle.x_, rectangle.y_,
                                      rectangle.width_, rectangle.height_},
                            Color{top.r_, top.g_, top.b_, top.a_},
                            Color{bottom.r_, bottom.g_, bottom.b_, bottom.a_},
                            Color{bottom.r_, bottom.g_, bottom.b_, bottom.a_},
                            Color{top.r_, top.g_, top.b_, top.a_});
}

void DrawCircleWrapped(float centerX, float centerY, float radius,
                       WrappedColor color) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    DrawCircle(centerX, centerY, radius, raylib_color);
}

void DrawCircleSectorWrapped(WrappedVector2 center, float radius,
                             float start_angle, float end_angle,
                             WrappedColor color) {
    DrawCircleSector(Vector2{center.x_, center.y_}, radius, start_angle,
                     end_angle, kArcSegments,
                     Color{color.r_, color.g_, color.b_, color.a_});
}

void DrawRingWrapped(WrappedVector2 center, float inner_radius,
                     float outer_radius, float start_angle, float end_angle,
                     WrappedColor color) {
    DrawRing(Vector2{center.x_, center.y_}, inner_radius, outer_radius,
             start_angle, end_angle, kArcSegments,
             Color{color.r_, color.g_, color.b_, color.a_});
}

void DrawTriangleWrapped(WrappedVector2 v1, WrappedVector2 v2,
                         WrappedVector2 v3, WrappedColor color) {
    // raylib only fills triangles given counter-clockwise (as seen on
    // screen): swap two vertices when they turn the other way.
    const float kCross =
        (v2.x_ - v1.x_) * (v3.y_ - v1.y_) - (v2.y_ - v1.y_) * (v3.x_ - v1.x_);
    if (kCross > 0.0F) {
        std::swap(v2, v3);
    }
    DrawTriangle(Vector2{v1.x_, v1.y_}, Vector2{v2.x_, v2.y_},
                 Vector2{v3.x_, v3.y_},
                 Color{color.r_, color.g_, color.b_, color.a_});
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
                       float spacing, WrappedColor color, FontWeight weight) {
    Color raylib_color = {color.r_, color.g_, color.b_, color.a_};
    Vector2 position = {x, y};
    DrawTextEx(GetFontWrapped(weight), text, position, fontSize, spacing,
               raylib_color);
}

float MeasureTextWrapped(const char *text, int fontSize) {
    return static_cast<float>(MeasureText(text, fontSize));
}

float MeasureTextExWrapped(const char *text, int fontSize, float spacing,
                           FontWeight weight) {
    return MeasureTextEx(GetFontWrapped(weight), text,
                         static_cast<float>(fontSize), spacing)
        .x;
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