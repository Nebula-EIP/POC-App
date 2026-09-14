/**
 * @file loader.tcc
 * @brief ModuleLoader's templated methods implementation.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 13-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#pragma once

// For IDE only
#include "loader.hpp"

namespace core {

template <typename Capability>
const Capability *ModuleLoader::resolve(IModule *instance) noexcept {
    static_assert(std::is_base_of_v<ICapability, Capability>,
                  "Capability must derive from core::ICapability");

    if (instance == nullptr) {
        return nullptr;
    }
    if (Capability *found = instance->capability<Capability>()) {
        return found;
    }
    // The two mandatory capabilities have dedicated accessors, so modules are
    // not required to also expose them through capability().
    if constexpr (std::is_same_v<Capability, capa::ITypeListCapability>) {
        return instance->types();
    } else if constexpr (std::is_same_v<Capability,
                                        capa::INodeListCapability>) {
        return instance->nodes();
    } else {
        return nullptr;
    }
}

template <typename Capability>
std::span<const Capability *> ModuleLoader::capabilities() {
    static_assert(std::is_base_of_v<ICapability, Capability>,
                  "Capability must derive from core::ICapability");

    auto found = std::make_shared<std::vector<const Capability *>>();
    found->reserve(_entries.size());
    for (const std::unique_ptr<Entry> &entry : _entries) {
        const Capability *capability =
            resolve<Capability>(entry->instance.get());
        if (capability != nullptr) {
            found->push_back(capability);
        }
    }

    // Keeping the vector alive in the loader is what makes the returned span
    // usable; it is replaced on the next call for the same capability type.
    _capability_cache[std::type_index(typeid(Capability))] = found;
    return std::span<const Capability *>(*found);
}

}  // namespace core
