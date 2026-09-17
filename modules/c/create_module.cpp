/**
 * @file create_module.cpp
 * @brief Hook used to retreive the C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 14-09-2026
 */

#include "modules/create_module.hpp"
#include "module.hpp"

extern "C" NEBULA_MODULE_EXPORT core::IModule *CreateModule() {
    return new CModule();
}
