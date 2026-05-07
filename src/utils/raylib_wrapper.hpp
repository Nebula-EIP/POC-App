#include <raylib.h>
#include <utility>

namespace utils
{
    #define LIGHTGRAY  CLITERAL(WrappedColor){ 200, 200, 200, 255 }   // Light Gray
    #define GRAY       CLITERAL(WrappedColor){ 130, 130, 130, 255 }   // Gray
    #define DARKGRAY   CLITERAL(WrappedColor){ 80, 80, 80, 255 }      // Dark Gray
    #define YELLOW     CLITERAL(WrappedColor){ 253, 249, 0, 255 }     // Yellow
    #define GOLD       CLITERAL(WrappedColor){ 255, 203, 0, 255 }     // Gold
    #define ORANGE     CLITERAL(WrappedColor){ 255, 161, 0, 255 }     // Orange
    #define PINK       CLITERAL(WrappedColor){ 255, 109, 194, 255 }   // Pink
    #define RED        CLITERAL(WrappedColor){ 230, 41, 55, 255 }     // Red
    #define MAROON     CLITERAL(WrappedColor){ 190, 33, 55, 255 }     // Maroon
    #define GREEN      CLITERAL(WrappedColor){ 0, 228, 48, 255 }      // Green
    #define LIME       CLITERAL(WrappedColor){ 0, 158, 47, 255 }      // Lime
    #define DARKGREEN  CLITERAL(WrappedColor){ 0, 117, 44, 255 }      // Dark Green
    #define SKYBLUE    CLITERAL(WrappedColor){ 102, 191, 255, 255 }   // Sky Blue
    #define BLUE       CLITERAL(WrappedColor){ 0, 121, 241, 255 }     // Blue
    #define DARKBLUE   CLITERAL(WrappedColor){ 0, 82, 172, 255 }      // Dark Blue
    #define PURPLE     CLITERAL(WrappedColor){ 200, 122, 255, 255 }   // Purple
    #define VIOLET     CLITERAL(WrappedColor){ 135, 60, 190, 255 }    // Violet
    #define DARKPURPLE CLITERAL(WrappedColor){ 112, 31, 126, 255 }    // Dark Purple
    #define BEIGE      CLITERAL(WrappedColor){ 211, 176, 131, 255 }   // Beige
    #define BROWN      CLITERAL(WrappedColor){ 127, 106, 79, 255 }    // Brown
    #define DARKBROWN  CLITERAL(WrappedColor){ 76, 63, 47, 255 }      // Dark Brown

    #define WHITE      CLITERAL(WrappedColor){ 255, 255, 255, 255 }   // White
    #define BLACK      CLITERAL(WrappedColor){ 0, 0, 0, 255 }         // Black
    #define BLANK      CLITERAL(WrappedColor){ 0, 0, 0, 0 }           // Blank (Transparent)
    #define MAGENTA    CLITERAL(WrappedColor){ 255, 0, 255, 255 }     // Magenta

    typedef struct WrappedColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    } WrappedColor;

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
    void DrawRectangleWrapped(float x, float y, float width, float height, WrappedColor color);
    void DrawRectangleLinesWrapped(float x, float y, float width, float height, WrappedColor color);
    void DrawCircleWrapped(float centerX, float centerY, float radius, WrappedColor color);
    void DrawLineWrapped(WrappedVector2 start, WrappedVector2 end, float thick, WrappedColor color);
    void DrawLineBezierWrapped(WrappedVector2 start, WrappedVector2 end, float thick, WrappedColor color);
    void DrawTextWrapped(const char *text, float x, float y, int fontSize, WrappedColor color);

    // Input functions
    bool isRightClicked();
    bool isLeftClicked();
    bool isRightDown();
    bool isLeftDown();

    // Collision functions

    bool CheckCollisionRecsWrapped(WrappedRectangle rec1, WrappedRectangle rec2);
    bool CheckCollisionCirclesWrapped(WrappedCircle circle1, WrappedCircle circle2);
    bool CheckCollisionCircleRecWrapped(WrappedCircle circle, WrappedRectangle rec);
    bool CheckCollisionCircleLineWrapped(WrappedCircle circle, WrappedVector2 start, WrappedVector2 end);
    bool CheckCollisionPointRecWrapped(WrappedVector2 point, WrappedRectangle rec);
    bool CheckCollisionPointCircleWrapped(WrappedVector2 point, WrappedCircle circle);
    bool CheckCollisionPointTriangleWrapped(WrappedVector2 point, WrappedVector2 v1, WrappedVector2 v2, WrappedVector2 v3);
    bool CheckCollisionPointLineWrapped(WrappedVector2 point, WrappedVector2 start, WrappedVector2 end, int threshold);
    bool CheckCollisionPointPolyWrapped(WrappedVector2 point, WrappedVector2 *vertices, int vertexCount);
    bool CheckCollisionLinesWrapped(WrappedVector2 start1, WrappedVector2 end1, WrappedVector2 start2, WrappedVector2 end2, WrappedVector2 *collisionPoint);
    WrappedRectangle GetCollisionRecWrapped(WrappedRectangle rec1, WrappedRectangle rec2);

}