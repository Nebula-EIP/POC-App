/**
 * @file module_validation_exception.hpp
 * @brief Defines exceptions related to module validation.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 27-08-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 27-08-2026
 */

#pragma once

#include "module_exception.hpp"

namespace core {

/**
 * @brief Thrown when a module fails validation.
 */
class ModuleValidationException : public ModuleException {
public:
    using ModuleException::ModuleException;
};

// =======================================

/**
 * @brief Thrown when a module is invalid or does not meet the required
 * interface.
 */
class InvalidModuleException : public ModuleValidationException {
public:
    using ModuleValidationException::ModuleValidationException;
};

/**
 * @brief Thrown when a module's version is incompatible with the core or other
 * modules.
 */
class InvalidModuleVersionException : public ModuleValidationException {
public:
    using ModuleValidationException::ModuleValidationException;
};

/**
 * @brief Thrown when a module's dependencies are not satisfied.
 */
class IncompatibleModuleException : public ModuleValidationException {
public:
    using ModuleValidationException::ModuleValidationException;
};

} // namespace core