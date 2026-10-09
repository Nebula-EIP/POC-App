/**
 * @file grid.hpp
 * @brief Adaptive canvas grid: picks which grid levels to draw for a given
 * zoom, Blender style (a fine grid nested in a coarse one)
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 09-10-2026
 */

#pragma once

namespace render {

/// Spacing of the reference grid level, in world units.
inline constexpr float kGridBaseSpacing = 50.0F;
/// Number of fine cells per coarse cell (each level is this much larger).
inline constexpr int kGridSubdivisions = 5;
/// Below this on-screen spacing (pixels) a level is too dense to be drawn.
inline constexpr float kGridMinScreenSpacing = 8.0F;

/**
 * @brief The two grid levels to draw at a given zoom.
 *
 * Levels are kGridBaseSpacing * kGridSubdivisions^n (n can be negative).
 * The fine level is the smallest one at least kGridMinScreenSpacing pixels
 * apart; the coarse level is the next one up. The fine level fades in as it
 * spreads out, so that when it reaches the coarse level's density the levels
 * shift by one without any visible jump.
 */
struct GridLevels {
    float fine_spacing_;    ///< World units between fine lines.
    float coarse_spacing_;  ///< World units between coarse lines.
    float fine_alpha_;      ///< Opacity of the fine lines, in [0, 1].
};

/**
 * @brief Computes the grid levels to draw for a zoom level.
 *
 * @param zoom The camera zoom (screen pixels per world unit). A non positive
 * value yields the base level with invisible fine lines.
 *
 * @return The fine and coarse levels and the fine lines' opacity.
 */
[[nodiscard]] GridLevels ComputeGridLevels(float zoom) noexcept;

}  // namespace render
