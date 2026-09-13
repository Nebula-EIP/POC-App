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
    unsigned char r_;
    unsigned char g_;
    unsigned char b_;
    unsigned char a_;
} WrappedColor;

// Inline constexpr color constants (use utils::GRAY etc.)
inline constexpr WrappedColor kLightgray = {200, 200, 200, 255};
inline constexpr WrappedColor kGray = {130, 130, 130, 255};
inline constexpr WrappedColor kDarkgray = {80, 80, 80, 255};
inline constexpr WrappedColor kYellow = {253, 249, 0, 255};
inline constexpr WrappedColor kGold = {255, 203, 0, 255};
inline constexpr WrappedColor kOrange = {255, 161, 0, 255};
inline constexpr WrappedColor kPink = {255, 109, 194, 255};
inline constexpr WrappedColor kRed = {230, 41, 55, 255};
inline constexpr WrappedColor kMaroon = {190, 33, 55, 255};
inline constexpr WrappedColor kGreen = {0, 228, 48, 255};
inline constexpr WrappedColor kLime = {0, 158, 47, 255};
inline constexpr WrappedColor kDarkgreen = {0, 117, 44, 255};
inline constexpr WrappedColor kSkyblue = {102, 191, 255, 255};
inline constexpr WrappedColor kBlue = {0, 121, 241, 255};
inline constexpr WrappedColor kDarkblue = {0, 82, 172, 255};
inline constexpr WrappedColor kPurple = {200, 122, 255, 255};
inline constexpr WrappedColor kViolet = {135, 60, 190, 255};
inline constexpr WrappedColor kDarkpurple = {112, 31, 126, 255};
inline constexpr WrappedColor kBeige = {211, 176, 131, 255};
inline constexpr WrappedColor kBrown = {127, 106, 79, 255};
inline constexpr WrappedColor kDarkbrown = {76, 63, 47, 255};
inline constexpr WrappedColor kWhite = {255, 255, 255, 255};
inline constexpr WrappedColor kBlack = {0, 0, 0, 255};
inline constexpr WrappedColor kBlank = {0, 0, 0, 0};
inline constexpr WrappedColor kMagenta = {255, 0, 255, 255};

typedef struct WrappedVector2 {
    float x_;
    float y_;
} WrappedVector2;

typedef struct WrappedRectangle {
    float x_;
    float y_;
    float width_;
    float height_;
} WrappedRectangle;

typedef struct WrappedCircle {
    float center_x_;
    float center_y_;
    float radius_;
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
bool IsRightClicked();
bool IsLeftClicked();
bool IsRightDown();
bool IsLeftDown();

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