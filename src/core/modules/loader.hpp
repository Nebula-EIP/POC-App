/**
 * @file loader.hpp
 * @brief Module loader interface. The module loader is responsible for loading
 * and unloading modules.
 *
 * @author Created by JeanBizeul
 * @date Created on 02-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 02-09f-2026
 */

#pragma once

#include <filesystem>
#include <vector>

#include "module.hpp"

namespace core {

/**
 * @brief The module loader is responsible for loading and unloading modules.
 */
class ModuleLoader {
   public:
    ModuleLoader() = default;
    ~ModuleLoader() = default;

    /**
     * @brief Load a module from a shared library.
     *
     * @param path The path to the shared library.
     *
     * @return The id of the loaded module, or 0 if an error occurred.
     */
    ModuleId load(std::filesystem::path path);

    /**
     * @brief Unload a module by its id.
     *
     * @param id The id of the module to unload.
     *
     * @return true if the module was successfully unloaded, false otherwise.
     */
    bool unload(ModuleId);

    /**
     * @brief Unload a module by its name.
     *
     * @param name The name of the module to unload, mendatory.
     *
     * @return true if the module was successfully unloaded,false otherwise.
     */
    bool unload(std::string_view name);

    /**
     * @brief Unload all modules.
     */
    void unloadAll();

    /**
     * @brief Get a module by its id.
     *
     * @param id Id of the module, mendatory.
     *
     * @return A pointer to the requested module, nullptr if not found.
     */
    IModule *module(ModuleId id) noexcept;

    /**
     * @brief Get a module by its name.
     *
     * @param name Name of the module, mendatory.
     *
     * @return A pointer to the requested module, nullptr if not found.
     */
    IModule *module(std::string_view name) noexcept;

    /**
     * @brief Get a const module by its id.
     *
     * @param id Id of the module, mendatory.
     *
     * @return A const pointer to the requested module, nullptr if not found.
     */
    const IModule *module(ModuleId id) const noexcept;

    /**
     * @brief Get a const module by its name.
     *
     * @param name Name of the module, mendatory.
     *
     * @return A const pointer to the requested module, nullptr if not found.
     */
    const IModule *module(std::string_view name) const noexcept;

    /**
     * @brief Get a list of all modules.
     *
     * @return A list of pointers to all modules.
     */
    std::span<const IModule *const> modules() noexcept;

    /**
     * @brief This method retreives all capabilities of a certain type.
     * To get one from a specific module, ask the module itself.
     *
     * @return A list of pointers to all capabilities of the asked type.
     */
    template <typename Capability>
    std::span<const Capability *> capabilities();

   private:
    std::vector<std::unique_ptr<IModule>> _modules;
};

}  // namespace core
