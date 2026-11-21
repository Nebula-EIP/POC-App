/**
 * @file validation_capability.cpp
 * @brief Implementation of helper queries on core::capa::ValidationResult.
 *
 * @author Created by mathys-f
 * @date Created on 15-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 21-11-2026
 */

#include "modules/capabilities/validation_capability.hpp"

#include <algorithm>

namespace core::capa {

bool ValidationResult::HasTypeError() const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ ==
                   ValidationError::ErrorType::kIncompatiblePinTypes;
        });
}

bool ValidationResult::HasValueError() const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ == ValidationError::ErrorType::kIncompatibleValue;
        });
}

bool ValidationResult::HasUnsupportedNodeError() const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ == ValidationError::ErrorType::kUnsupportedNode;
        });
}

bool ValidationResult::HasPinConnectivityError() const noexcept {
    return std::any_of(
        errors_.begin(), errors_.end(), [](const ValidationError &err) {
            return err.type_ ==
                       ValidationError::ErrorType::kMissingRequiredInputPin ||
                   err.type_ ==
                       ValidationError::ErrorType::kMissingRequiredOutputPin;
        });
}

}  // namespace core::capa
