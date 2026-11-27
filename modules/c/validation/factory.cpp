/**
 * @file factory.cpp
 * @brief Factory building the graph validation capability of the C module.
 *
 * @author Created by mathys-f
 * @date Created on 21-11-2026
 *
 * @author Last modified by mathys-f
 * @date Last modified on 24-11-2026
 */

#include "modules_inits.hpp"
#include "validation/c_validation_capability.hpp"

core::capa::IValidationCapability *CreateValidationCapa() {
    return new c_module::CValidationCapability();
}
