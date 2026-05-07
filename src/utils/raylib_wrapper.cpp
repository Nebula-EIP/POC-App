#include "raylib_wrapper.hpp"

namespace utils
{
    void InitRaylib(int width, int height, const char *title)
    {
        InitWindow(width, height, title);
    }

    void SetFPS(int fps)
    {
        SetTargetFPS(fps);
    }

    void CloseRaylib()
    {
        CloseWindow();
    }

    void ClearScreen()
    {
        ClearBackground(RAYWHITE);
    }

    WrappedVector2 GetCursorPositionWrapped()
    {
        Vector2 pos = GetMousePosition();
        WrappedVector2 wrappedPos = {pos.x, pos.y};

        return wrappedPos;
    }

    // Draw functions

    void DrawRectangleWrapped(float x, float y, float width, float height, WrappedColor color)
    {
        Color raylibColor = {color.r, color.g, color.b, color.a};
        DrawRectangle(x, y, width, height, raylibColor);
    }

    void DrawRectangleLinesWrapped(float x, float y, float width, float height, WrappedColor color)
    {
        Color raylibColor = {color.r, color.g, color.b, color.a};
        DrawRectangleLines(x, y, width, height, raylibColor);
    }

    void DrawCircleWrapped(float centerX, float centerY, float radius, WrappedColor color)
    {
        Color raylibColor = {color.r, color.g, color.b, color.a};
        DrawCircle(centerX, centerY, radius, raylibColor);
    }

    void DrawLineWrapped(WrappedVector2 start, WrappedVector2 end, float thick, WrappedColor color)
    {
        Color raylibColor = {color.r, color.g, color.b, color.a};
        Vector2 startVec = {start.x, start.y};
        Vector2 endVec = {end.x, end.y};
    
        DrawLineEx(startVec, endVec, thick, raylibColor);
    }

    void DrawLineBezierWrapped(WrappedVector2 start, WrappedVector2 end, float thick, WrappedColor color)
    {
        Color raylibColor = {color.r, color.g, color.b, color.a};
        Vector2 startVec = {start.x, start.y};
        Vector2 endVec = {end.x, end.y};
    
        DrawLineBezier(startVec, endVec, thick, raylibColor);
    }

    void DrawTextWrapped(const char *text, float x, float y, int fontSize, WrappedColor color)
    {
        Color raylibColor = {color.r, color.g, color.b, color.a};
        DrawText(text, x, y, fontSize, raylibColor);
    }

    // Input functions
    
    bool isRightClicked()
    {
        return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    }

    bool isLeftClicked()
    {
        return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    bool isRightDown()
    {
        return IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    }

    bool isLeftDown()
    {
        return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    }

    // Collision functions

    bool CheckCollisionRecsWrapped(WrappedRectangle rec1, WrappedRectangle rec2)
    {
        Rectangle rec1Ray = {rec1.x, rec1.y, rec1.width, rec1.height};
        Rectangle rec2Ray = {rec2.x, rec2.y, rec2.width, rec2.height};
    
        return CheckCollisionRecs(rec1Ray, rec2Ray);
    }

    bool CheckCollisionCirclesWrapped(WrappedCircle circle1, WrappedCircle circle2)
    {
        Vector2 center1Vec = {circle1.centerX, circle1.centerY};
        Vector2 center2Vec = {circle2.centerX, circle2.centerY};
    
        return CheckCollisionCircles(center1Vec, circle1.radius, center2Vec, circle2.radius);
    }

    bool CheckCollisionCircleRecWrapped(WrappedCircle circle, WrappedRectangle rec)
    {
        Vector2 centerVec = {circle.centerX, circle.centerY};
        Rectangle recRay = {rec.x, rec.y, rec.width, rec.height};
    
        return CheckCollisionCircleRec(centerVec, circle.radius, recRay);
    }

    bool CheckCollisionCircleLineWrapped(WrappedCircle circle, WrappedVector2 start, WrappedVector2 end)
    {
        Vector2 centerVec = {circle.centerX, circle.centerY};
        Vector2 startVec = {start.x, start.y};
        Vector2 endVec = {end.x, end.y};
    
        return CheckCollisionCircleLine(centerVec, circle.radius, startVec, endVec);
    }

    bool CheckCollisionPointRecWrapped(WrappedVector2 point, WrappedRectangle rec)
    {
        Vector2 pointVec = {point.x, point.y};
        Rectangle recRay = {rec.x, rec.y, rec.width, rec.height};
    
        return CheckCollisionPointRec(pointVec, recRay);
    }

    bool CheckCollisionPointCircleWrapped(WrappedVector2 point, WrappedCircle circle)
    {
        Vector2 pointVec = {point.x, point.y};
        Vector2 centerVec = {circle.centerX, circle.centerY};
    
        return CheckCollisionPointCircle(pointVec, centerVec, circle.radius);
    }

    bool CheckCollisionPointTriangleWrapped(WrappedVector2 point, WrappedVector2 v1, WrappedVector2 v2, WrappedVector2 v3)
    {
        Vector2 pointVec = {point.x, point.y};
        Vector2 v1Vec = {v1.x, v1.y};
        Vector2 v2Vec = {v2.x, v2.y};
        Vector2 v3Vec = {v3.x, v3.y};
    
        return CheckCollisionPointTriangle(pointVec, v1Vec, v2Vec, v3Vec);
    }

    bool CheckCollisionPointLineWrapped(WrappedVector2 point, WrappedVector2 start, WrappedVector2 end, int threshold)
    {
        Vector2 pointVec = {point.x, point.y};
        Vector2 startVec = {start.x, start.y};
        Vector2 endVec = {end.x, end.y};
    
        return CheckCollisionPointLine(pointVec, startVec, endVec, threshold);
    }

    bool CheckCollisionPointPolyWrapped(WrappedVector2 point, WrappedVector2 *vertices, int vertexCount)
    {
        Vector2 pointVec = {point.x, point.y};
        Vector2 *verticesVec = new Vector2[vertexCount];
        for (int i = 0; i < vertexCount; i++) {
            verticesVec[i] = {vertices[i].x, vertices[i].y};
        }
    
        bool result = CheckCollisionPointPoly(pointVec, verticesVec, vertexCount);
        delete[] verticesVec;
        return result;
    }

    bool CheckCollisionLinesWrapped(WrappedVector2 start1, WrappedVector2 end1, WrappedVector2 start2, WrappedVector2 end2, WrappedVector2 *collisionPoint)
    {
        Vector2 start1Vec = {start1.x, start1.y};
        Vector2 end1Vec = {end1.x, end1.y};
        Vector2 start2Vec = {start2.x, start2.y};
        Vector2 end2Vec = {end2.x, end2.y};

        return CheckCollisionLines(start1Vec, end1Vec, start2Vec, end2Vec, (Vector2 *)collisionPoint);    
    }

    WrappedRectangle GetCollisionRecWrapped(WrappedRectangle rec1, WrappedRectangle rec2)
    {
        Rectangle rec1Ray = {rec1.x, rec1.y, rec1.width, rec1.height};
        Rectangle rec2Ray = {rec2.x, rec2.y, rec2.width, rec2.height};
    
        Rectangle collisionRec = GetCollisionRec(rec1Ray, rec2Ray);
        return {collisionRec.x, collisionRec.y, collisionRec.width, collisionRec.height};
    }
}