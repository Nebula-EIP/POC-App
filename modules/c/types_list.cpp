/**
 * @file init_types.cpp
 * @brief Contains the constructor which builds the capa containing all types
 * from C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 20-09-2026
 */

#include "modules_inits.hpp"

core::capa::TypeListCapability *CreateTypeListCapa() {
    auto *types = new core::capa::TypeListCapability();
    types->RegisterType("int");
    types->RegisterType("double");
    types->RegisterType("bool");
    types->RegisterType("char");
    return types;
}
