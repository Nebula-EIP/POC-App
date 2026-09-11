/**
 * @file module.tcc
 * @brief Module's templated methods implementation.
 *
 * @author Created by JeanBizeul
 * @date Created on 02-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 02-09f-2026
 */

#pragma once

// For IDE  only
#include "module.hpp"

namespace core {

// These functions allow safer use of the internal capability() methods,
// by using the typeid of the template parameter instead of a std::type_index

template <typename T>
T *IModule::capability() noexcept {
    static_assert(std::is_base_of_v<ICapability, T>);
    return static_cast<T *>(capability(typeid(T)));
}

template <typename T>
const T *IModule::capability() const noexcept {
    static_assert(std::is_base_of_v<ICapability, T>);
    return static_cast<const T *>(capability(typeid(T)));
}

}  // namespace core
