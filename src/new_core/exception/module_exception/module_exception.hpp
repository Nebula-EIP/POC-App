/**
 * @file module_exception.hpp
 * @brief Defines exceptions related to module management.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 27-08-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 27-08-2026
 */

#pragma once

#include "exception/exception.hpp"

namespace core {

/**
 * @brief Base exception for errors related to module management.
 */
class ModuleException : public Exception {
public:
    using Exception::Exception;
};

/**
 * @brief Thrown when a module fails to initialize.
 */
class ModuleInitializationException : public ModuleException {
public:
    using ModuleException::ModuleException;
};

/**
 * @brief Thrown when a module cannot be found or is invalid.
 */
class ModuleNotFoundException : public ModuleException {
public:
    using ModuleException::ModuleException;
};

/**
 * @brief Thrown when a module is already loaded and cannot be loaded again.
 */
class ModuleAlreadyLoadedException : public ModuleException {
public:
    using ModuleException::ModuleException;
};

} // namespace core