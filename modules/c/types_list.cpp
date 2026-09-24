/**
 * @file init_types.cpp
 * @brief Contains the constructor which builds the capa containing all types
 * from C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 24-09-2026
 */

#include "modules_inits.hpp"

namespace {

/// Names of the C types, in the order the core registers them.
const std::vector<std::string_view> kTypesNames = {
    "void",    ///> Void/Null
    "int",     ///> Integer
    "float",   ///> Floating point number
    "bool",    ///> Boolean
    "char",    ///> Character - uint8_t
    "string",  ///> string pointer
};

}  // namespace

core::capa::TypeListCapability *CreateTypeListCapa() {
    return new core::capa::TypeListCapability(kTypesNames);
}
