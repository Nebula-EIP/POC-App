/**
 * @file module.hpp
 * @brief Module interface. Modules are loaded by the module loader
 * and provide capabilities to the core.
 *
 * @author Created by JeanBizeul
 * @date Created on 02-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 02-09f-2026
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <typeindex>

#include "capabilities/node_list_capability.hpp"
#include "capabilities/type_list_capability.hpp"

namespace core {

class ICapability;

using ModuleId = uint32_t;

/**
 * @brief Modules will be loaded by the module loader class.
 * They contains a set of capabilities.
 */
class IModule {
   public:
    virtual ~IModule() = default;

    /**
     * @brief Get the name of the module.
     *
     * @return The name of the module.
     */
    virtual std::string_view name() const noexcept = 0;

    /**
     * @brief Get the author of the module.
     *
     * @return The author of the module.
     */
    virtual std::string_view author() const noexcept = 0;

    /**
     * @brief Get the description of the module.
     *
     * @return The description of the module.
     */
    virtual std::string_view description() const noexcept = 0;

    /**
     * @brief Version wrapper
     */
    struct Version {
        uint16_t major;
        uint16_t minor;
        uint16_t patch;
    };

    /**
     * @brief Get the version of the module.
     *
     * @return The version of the module.
     */
    virtual Version version() const noexcept = 0;

    /**
     * @brief Initialize the module
     *
     * @param id The id of the module, assigned by the core. Required.
     *
     * @return true if the module has been initialized, false if not
     */
    virtual bool initialize(ModuleId id) = 0;

    /**
     * @brief Ask the module to release all resources and prepare for shutdown.
     */
    virtual void shutdown() noexcept = 0;

    /**
     * @brief Get the id of the module.
     *
     * @return The id of the module.
     */
    virtual ModuleId id() const noexcept = 0;

    /// Mendatory capabilities

    /**
     * @brief Get the type list capability of the module.
     *
     * @return The type list capability of the module..
     */
    virtual capa::ITypeListCapability *types() = 0;

    /**
     * @brief Get the node list capability of the module.
     *
     * @return The node list capability of the module.
     */
    virtual capa::INodeListCapability *nodes() = 0;

    /**
     * @brief Get a capability by type.
     *
     * @param T The type of the capability to get.
     *
     * @return A pointer to the capability, or nullptr if not found.
     */
    template <typename T>
    T *capability() noexcept;

    /**
     * @brief Get a const capability by type.
     *
     * @param T The type of the capability to get.
     *
     * @return A const pointer to the capability, or nullptr if not found.
     */
    template <typename T>
    const T *capability() const noexcept;

   private:
    /**
     * @brief Get a capability by type.
     * Internal method, use the template version instead.
     *
     * @param type The type of the capability to get.
     *
     * @return A pointer to the capability, or nullptr if not found.
     */
    virtual ICapability *capability(std::type_index type) noexcept = 0;

    /**
     * @brief Get a const capability by type.
     * Internal method, use the template version instead.
     *
     * @param type The type of the capability to get.
     *
     * @return A const pointer to the capability, or nullptr if not found.
     */
    virtual const ICapability *capability(
        std::type_index type) const noexcept = 0;
};

}  // namespace core

// Retreive the implementation of the module template functions
#include "module.tcc"
