/**
 * @file grid_tests.cpp
 * @brief Tests for the adaptive grid level selection
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include <gtest/gtest.h>

#include <cmath>

#include "render/camera.hpp"
#include "render/grid.hpp"

namespace {

constexpr float kEpsilon = 0.001F;
constexpr auto kFactor = static_cast<float>(render::kGridSubdivisions);

// Sweeps the whole camera zoom range in small multiplicative steps.
template <typename Fn>
void ForEachZoom(Fn &&fn) {
    for (float zoom = render::Camera::kMinZoom;
         zoom <= render::Camera::kMaxZoom; zoom *= 1.01F) {
        fn(zoom);
    }
}

}  // namespace

TEST(GridTest, FineLevelStaysAboveMinimumDensity) {
    ForEachZoom([](float zoom) {
        const render::GridLevels kLevels = render::ComputeGridLevels(zoom);
        const float kOnScreen = kLevels.fine_spacing_ * zoom;
        EXPECT_GE(kOnScreen, render::kGridMinScreenSpacing - kEpsilon)
            << "zoom " << zoom;
        EXPECT_LT(kOnScreen, render::kGridMinScreenSpacing * kFactor + kEpsilon)
            << "zoom " << zoom;
    });
}

TEST(GridTest, CoarseLevelIsNextLevelUp) {
    ForEachZoom([](float zoom) {
        const render::GridLevels kLevels = render::ComputeGridLevels(zoom);
        EXPECT_NEAR(kLevels.coarse_spacing_, kLevels.fine_spacing_ * kFactor,
                    kEpsilon);
    });
}

TEST(GridTest, LevelsArePowersOfSubdivisionsTimesBase) {
    ForEachZoom([](float zoom) {
        const float kExponent =
            std::log(render::ComputeGridLevels(zoom).fine_spacing_ /
                     render::kGridBaseSpacing) /
            std::log(kFactor);
        EXPECT_NEAR(kExponent, std::round(kExponent), kEpsilon)
            << "zoom " << zoom;
    });
}

TEST(GridTest, FineAlphaIsAnOpacity) {
    ForEachZoom([](float zoom) {
        const float kAlpha = render::ComputeGridLevels(zoom).fine_alpha_;
        EXPECT_GE(kAlpha, 0.0F);
        EXPECT_LE(kAlpha, 1.0F);
    });
}

TEST(GridTest, FineLinesFadeInWhileZoomingIn) {
    const render::GridLevels kFar = render::ComputeGridLevels(1.0F);
    const render::GridLevels kNear = render::ComputeGridLevels(1.5F);
    ASSERT_FLOAT_EQ(kFar.fine_spacing_, kNear.fine_spacing_);
    EXPECT_GT(kNear.fine_alpha_, kFar.fine_alpha_);
}

TEST(GridTest, LevelShiftIsSeamless) {
    // Zoom at which the base level, as the fine level, reaches the coarse
    // density (inside the camera zoom range).
    const float kShift =
        render::kGridMinScreenSpacing * kFactor / render::kGridBaseSpacing;
    ASSERT_GT(kShift, render::Camera::kMinZoom);
    ASSERT_LT(kShift, render::Camera::kMaxZoom);
    const render::GridLevels kBefore =
        render::ComputeGridLevels(kShift * 0.999F);
    const render::GridLevels kAfter =
        render::ComputeGridLevels(kShift * 1.001F);

    // Before: fine lines almost fully visible.
    EXPECT_NEAR(kBefore.fine_spacing_, render::kGridBaseSpacing, kEpsilon);
    EXPECT_NEAR(kBefore.fine_alpha_, 1.0F, 0.01F);
    // After: they became the coarse level, and a new fine level starts
    // invisible.
    EXPECT_NEAR(kAfter.coarse_spacing_, kBefore.fine_spacing_, kEpsilon);
    EXPECT_NEAR(kAfter.fine_alpha_, 0.0F, 0.01F);
}

TEST(GridTest, DefaultZoomShowsBaseSpacingAsCoarseLevel) {
    const render::GridLevels kLevels = render::ComputeGridLevels(1.0F);
    EXPECT_FLOAT_EQ(kLevels.coarse_spacing_, render::kGridBaseSpacing);
    EXPECT_GT(kLevels.fine_alpha_, 0.0F);
}

TEST(GridTest, NonPositiveZoomFallsBackToBaseLevel) {
    for (const float kZoom : {0.0F, -1.0F}) {
        const render::GridLevels kLevels = render::ComputeGridLevels(kZoom);
        EXPECT_FLOAT_EQ(kLevels.fine_spacing_, render::kGridBaseSpacing);
        EXPECT_FLOAT_EQ(kLevels.fine_alpha_, 0.0F);
    }
}
