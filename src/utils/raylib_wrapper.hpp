#pragma once

#include <raylib.h>

#include <utility>

namespace utils {
// clang-format off
// Undef any raylib color macros so we can expose namespaced constants
#ifdef LIGHTGRAY
#undef LIGHTGRAY
#endif
#ifdef GRAY
#undef GRAY
#endif
#ifdef DARKGRAY
#undef DARKGRAY
#endif
#ifdef YELLOW
#undef YELLOW
#endif
#ifdef GOLD
#undef GOLD
#endif
#ifdef ORANGE
#undef ORANGE
#endif
#ifdef PINK
#undef PINK
#endif
#ifdef RED
#undef RED
#endif
#ifdef MAROON
#undef MAROON
#endif
#ifdef GREEN
#undef GREEN
#endif
#ifdef LIME
#undef LIME
#endif
#ifdef DARKGREEN
#undef DARKGREEN
#endif
#ifdef SKYBLUE
#undef SKYBLUE
#endif
#ifdef BLUE
#undef BLUE
#endif
#ifdef DARKBLUE
#undef DARKBLUE
#endif
#ifdef PURPLE
#undef PURPLE
#endif
#ifdef VIOLET
#undef VIOLET
#endif
#ifdef DARKPURPLE
#undef DARKPURPLE
#endif
#ifdef BEIGE
#undef BEIGE
#endif
#ifdef BROWN
#undef BROWN
#endif
#ifdef DARKBROWN
#undef DARKBROWN
#endif
#ifdef WHITE
#undef WHITE
#endif
#ifdef BLACK
#undef BLACK
#endif
#ifdef BLANK
#undef BLANK
#endif
#ifdef MAGENTA
#undef MAGENTA
#endif

// clang-format on

typedef struct WrappedColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} WrappedColor;

// Inline constexpr color constants (use utils::GRAY etc.)
inline constexpr WrappedColor LIGHTGRAY = {200, 200, 200, 255};
inline constexpr WrappedColor GRAY = {130, 130, 130, 255};
inline constexpr WrappedColor DARKGRAY = {80, 80, 80, 255};
inline constexpr WrappedColor YELLOW = {253, 249, 0, 255};
inline constexpr WrappedColor GOLD = {255, 203, 0, 255};
inline constexpr WrappedColor ORANGE = {255, 161, 0, 255};
inline constexpr WrappedColor PINK = {255, 109, 194, 255};
inline constexpr WrappedColor RED = {230, 41, 55, 255};
inline constexpr WrappedColor MAROON = {190, 33, 55, 255};
inline constexpr WrappedColor GREEN = {0, 228, 48, 255};
inline constexpr WrappedColor LIME = {0, 158, 47, 255};
inline constexpr WrappedColor DARKGREEN = {0, 117, 44, 255};
inline constexpr WrappedColor SKYBLUE = {102, 191, 255, 255};
inline constexpr WrappedColor BLUE = {0, 121, 241, 255};
inline constexpr WrappedColor DARKBLUE = {0, 82, 172, 255};
inline constexpr WrappedColor PURPLE = {200, 122, 255, 255};
inline constexpr WrappedColor VIOLET = {135, 60, 190, 255};
inline constexpr WrappedColor DARKPURPLE = {112, 31, 126, 255};
inline constexpr WrappedColor BEIGE = {211, 176, 131, 255};
inline constexpr WrappedColor BROWN = {127, 106, 79, 255};
inline constexpr WrappedColor DARKBROWN = {76, 63, 47, 255};
inline constexpr WrappedColor WHITE = {255, 255, 255, 255};
inline constexpr WrappedColor BLACK = {0, 0, 0, 255};
inline constexpr WrappedColor BLANK = {0, 0, 0, 0};
inline constexpr WrappedColor MAGENTA = {255, 0, 255, 255};

typedef struct WrappedVector2 {
    float x;
    float y;
} WrappedVector2;

typedef struct WrappedRectangle {
    float x;
    float y;
    float width;
    float height;
} WrappedRectangle;

typedef struct WrappedCircle {
    float centerX;
    float centerY;
    float radius;
} WrappedCircle;

void InitRaylib(int width, int height, const char *title);
void SetFPS(int fps);
void CloseRaylib();
void ClearScreen();
WrappedVector2 GetCursorPositionWrapped();

// Draw functions
void DrawRectangleWrapped(float x, float y, float width, float height,
                          WrappedColor color);
void DrawRectangleLinesWrapped(float x, float y, float width, float height,
                               WrappedColor color);
void DrawCircleWrapped(float centerX, float centerY, float radius,
                       WrappedColor color);
void DrawLineWrapped(WrappedVector2 start, WrappedVector2 end, float thick,
                     WrappedColor color);
void DrawLineBezierWrapped(WrappedVector2 start, WrappedVector2 end,
                           float thick, WrappedColor color);
void DrawTextWrapped(const char *text, float x, float y, int fontSize,
                     WrappedColor color);

// Input functions
bool isRightClicked();
bool isLeftClicked();
bool isRightDown();
bool isLeftDown();

// Collision functions

bool CheckCollisionRecsWrapped(WrappedRectangle rec1, WrappedRectangle rec2);
bool CheckCollisionCirclesWrapped(WrappedCircle circle1, WrappedCircle circle2);
bool CheckCollisionCircleRecWrapped(WrappedCircle circle, WrappedRectangle rec);
bool CheckCollisionCircleLineWrapped(WrappedCircle circle, WrappedVector2 start,
                                     WrappedVector2 end);
bool CheckCollisionPointRecWrapped(WrappedVector2 point, WrappedRectangle rec);
bool CheckCollisionPointCircleWrapped(WrappedVector2 point,
                                      WrappedCircle circle);
bool CheckCollisionPointTriangleWrapped(WrappedVector2 point, WrappedVector2 v1,
                                        WrappedVector2 v2, WrappedVector2 v3);
bool CheckCollisionPointLineWrapped(WrappedVector2 point, WrappedVector2 start,
                                    WrappedVector2 end, int threshold);
bool CheckCollisionPointPolyWrapped(WrappedVector2 point,
                                    WrappedVector2 *vertices, int vertexCount);
bool CheckCollisionLinesWrapped(WrappedVector2 start1, WrappedVector2 end1,
                                WrappedVector2 start2, WrappedVector2 end2);
WrappedRectangle GetCollisionRecWrapped(WrappedRectangle rec1,
                                        WrappedRectangle rec2);

}  // namespace utils