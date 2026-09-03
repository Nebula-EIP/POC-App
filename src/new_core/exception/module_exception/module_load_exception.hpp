/**
 * @file module_load_exception.hpp
 * @brief Defines exceptions related to module loading.
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
 * @brief Thrown when a module fails to load.
 */
class ModuleLoadException : public ModuleException {
   public:
    using ModuleException::ModuleException;
};

// ========================================

/**
 * @brief Thrown when a module file cannot be found.
 */
class ModuleFileNotFoundException : public ModuleLoadException {
   public:
    using ModuleLoadException::ModuleLoadException;
};

/**
 * @brief Thrown when a module file is invalid or cannot be loaded.
 */
class ModuleLoadFailedException : public ModuleLoadException {
   public:
    using ModuleLoadException::ModuleLoadException;
};

/**
 * @brief Thrown when a module does not contain the required entry point.
 */
class ModuleSymbolNotFoundException : public ModuleLoadException {
   public:
    using ModuleLoadException::ModuleLoadException;
};

}  // namespace core