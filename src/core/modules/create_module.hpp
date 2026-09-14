/**
 * @file create_module.hpp
 * @brief External function to create a new module instance.
 *
 * @author Created by JeanBizeul
 * @date Created on 02-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#pragma once

#include "module.hpp"

/**
 * @brief Marks the module factory as exported from the shared library.
 *
 * Windows only exports what is explicitly marked, so a module that does not
 * use this macro builds fine but is rejected by the loader with a
 * ModuleSymbolNotFoundException.
 */
#if defined(_WIN32)
#define NEBULA_MODULE_EXPORT __declspec(dllexport)
#else
#define NEBULA_MODULE_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {

/**
 * @brief Create a new module instance.
 *
 * The module is allocated by the library and destroyed by the core through the
 * IModule virtual destructor, before the library is closed. The library must
 * therefore be built against the same C++ runtime as the core.
 *
 * @return A pointer to the new module, or nullptr if creation failed.
 */
NEBULA_MODULE_EXPORT core::IModule *CreateModule();
}
