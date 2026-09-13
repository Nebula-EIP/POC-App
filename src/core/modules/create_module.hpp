/**
 * @file create_module.hpp
 * @brief External function to create a new module instance.
 *
 * @author Created by JeanBizeul
 * @date Created on 02-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 02-09-2026
 */

#pragma once

#include "module.hpp"

extern "C" {

/**
 * @brief Create a new module instance.
 *
 * @return A pointer to the new module, or nullptr if creation failed.
 */
core::IModule *CreateModule();
}
