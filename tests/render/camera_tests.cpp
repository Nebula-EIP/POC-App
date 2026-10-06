/**
 * @file camera_tests.cpp
 * @brief Tests for the camera class
 *
 * @author Created by NathanBezard
 * @date Created on 27-09-2026
 */

#include <gtest/gtest.h>

#include "render/camera.hpp"

namespace {

constexpr float kEpsilon = 0.001F;

void ExpectNear(utils::WrappedVector2 a, utils::WrappedVector2 b) {
    EXPECT_NEAR(a.x_, b.x_, kEpsilon);
    EXPECT_NEAR(a.y_, b.y_, kEpsilon);
}

}  // namespace

// No InitWindow() anywhere in this file: Camera never touches the window,
// or raylib at all beyond the WrappedVector2 type it's built on.

TEST(CameraTest, ScreenToWorldRoundTripAtDefaultState) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);

    const utils::WrappedVector2 kScreenPoint{123.0F, 456.0F};
    const utils::WrappedVector2 kWorld = camera.ScreenToWorld(kScreenPoint);
    const utils::WrappedVector2 kBackToScreen = camera.WorldToScreen(kWorld);

    ExpectNear(kBackToScreen, kScreenPoint);
}

TEST(CameraTest, RoundTripHoldsAfterPanAndZoom) {
    render::Camera camera;
    camera.SetViewport(1024.0F, 768.0F);

    camera.Pan(utils::WrappedVector2{40.0F, -15.0F});
    camera.ZoomAt(utils::WrappedVector2{300.0F, 200.0F}, 3.0F);

    const utils::WrappedVector2 kScreenPoint{300.0F, 200.0F};
    const utils::WrappedVector2 kWorld = camera.ScreenToWorld(kScreenPoint);
    const utils::WrappedVector2 kBackToScreen = camera.WorldToScreen(kWorld);

    ExpectNear(kBackToScreen, kScreenPoint);
}

TEST(CameraTest, ZoomIsClampedToTwentyFivePercentMinimum) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);

    for (int i = 0; i < 50; ++i) {
        camera.ZoomAt(utils::WrappedVector2{400.0F, 300.0F}, -1.0F);
    }

    EXPECT_NEAR(camera.Zoom(), render::Camera::kMinZoom, kEpsilon);
}

TEST(CameraTest, ZoomIsClampedToFourHundredPercentMaximum) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);

    for (int i = 0; i < 50; ++i) {
        camera.ZoomAt(utils::WrappedVector2{400.0F, 300.0F}, 1.0F);
    }

    EXPECT_NEAR(camera.Zoom(), render::Camera::kMaxZoom, kEpsilon);
}

TEST(CameraTest, ZoomAtKeepsAnchorWorldPointUnderCursor) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);

    const utils::WrappedVector2 kAnchor{250.0F, 180.0F};
    const utils::WrappedVector2 kWorldUnderAnchorBefore =
        camera.ScreenToWorld(kAnchor);

    camera.ZoomAt(kAnchor, 2.0F);

    const utils::WrappedVector2 kScreenOfSameWorldPoint =
        camera.WorldToScreen(kWorldUnderAnchorBefore);
    ExpectNear(kScreenOfSameWorldPoint, kAnchor);
}
TEST(CameraTest, SetZoomKeepsTheAnchorInPlace) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);
    camera.ZoomAt(utils::WrappedVector2{100.0F, 100.0F}, 5.0F);
    camera.Pan(utils::WrappedVector2{-60.0F, 25.0F});

    const utils::WrappedVector2 kAnchor{400.0F, 315.0F};
    const utils::WrappedVector2 kWorldBefore = camera.ScreenToWorld(kAnchor);
    camera.SetZoom(1.0F, kAnchor);

    EXPECT_NEAR(camera.Zoom(), 1.0F, kEpsilon);
    ExpectNear(camera.ScreenToWorld(kAnchor), kWorldBefore);
}

TEST(CameraTest, SetZoomIsClamped) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);

    camera.SetZoom(100.0F, utils::WrappedVector2{0.0F, 0.0F});
    EXPECT_NEAR(camera.Zoom(), render::Camera::kMaxZoom, kEpsilon);
    camera.SetZoom(0.0F, utils::WrappedVector2{0.0F, 0.0F});
    EXPECT_NEAR(camera.Zoom(), render::Camera::kMinZoom, kEpsilon);
}

TEST(CameraTest, CenterOnShowsTheWorldPointAtTheAnchor) {
    render::Camera camera;
    camera.SetViewport(800.0F, 600.0F);
    camera.ZoomAt(utils::WrappedVector2{300.0F, 200.0F}, 2.0F);
    const float kZoom = camera.Zoom();

    const utils::WrappedVector2 kWorld{1234.0F, -567.0F};
    const utils::WrappedVector2 kAnchor{400.0F, 315.0F};
    camera.CenterOn(kWorld, kAnchor);

    ExpectNear(camera.WorldToScreen(kWorld), kAnchor);
    EXPECT_NEAR(camera.Zoom(), kZoom, kEpsilon);
}
