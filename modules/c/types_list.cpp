/**
 * @file init_types.cpp
 * @brief Contains the constructor which builds the capa containing all types from C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 14-09-2026
 */

#include "modules_inits.hpp"

const std::vector<std::string_view> kTypesNames = {
    "void",     ///> Void/Null
    "int",      ///> Integer
    "float",    ///> Floating point number
    "bool",     ///> Boolean
    "char",     ///> Character - uint8_t
    "string",   ///> string pointer
};

core::capa::TypeListCapability *CreateTypeListCapa()
{
    return new core::capa::TypeListCapability(kTypesNames);
}
