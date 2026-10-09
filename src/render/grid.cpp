/**
 * @file grid.cpp
 * @brief Implementation of the adaptive canvas grid level selection
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 09-10-2026
 */

#include "grid.hpp"

#include <algorithm>
#include <cmath>

namespace render {

GridLevels ComputeGridLevels(float zoom) noexcept {
    const auto kFactor = static_cast<float>(kGridSubdivisions);

    if (zoom <= 0.0F) {
        return GridLevels{
            .fine_spacing_ = kGridBaseSpacing,
            .coarse_spacing_ = kGridBaseSpacing * kFactor,
            .fine_alpha_ = 0.0F,
        };
    }

    const float kLogFactor = std::log(kFactor);
    // Smallest n such that kGridBaseSpacing * kFactor^n * zoom >= min spacing.
    const float kLevel =
        std::ceil(std::log(kGridMinScreenSpacing / (kGridBaseSpacing * zoom)) /
                  kLogFactor);
    const float kFineSpacing = kGridBaseSpacing * std::pow(kFactor, kLevel);

    // 0 when the fine lines are at the minimum density, 1 when they are as
    // far apart as the coarse lines will be once the levels shift.
    const float kProgress =
        std::log(kFineSpacing * zoom / kGridMinScreenSpacing) / kLogFactor;

    return GridLevels{
        .fine_spacing_ = kFineSpacing,
        .coarse_spacing_ = kFineSpacing * kFactor,
        .fine_alpha_ = std::clamp(kProgress, 0.0F, 1.0F),
    };
}

}  // namespace render
