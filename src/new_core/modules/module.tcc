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
