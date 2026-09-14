/**
 * @file loader.hpp
 * @brief Module loader interface. The module loader is responsible for loading
 * and unloading modules.
 *
 * @author Created by JeanBizeul
 * @date Created on 02-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "module.hpp"
#include "shared_library.hpp"

namespace core {

/// @brief Name of the factory symbol every module must export with C linkage.
/// Keep in sync with create_module.hpp.
inline constexpr const char *kCreateModuleSymbol = "CreateModule";

/// @brief Signature of the factory symbol every module must export.
using CreateModuleFunction = IModule *(*)();

/**
 * @brief The module loader is responsible for loading and unloading modules.
 *
 * ## Error reporting convention
 *
 * The loader uses exactly two conventions, and never mixes them:
 *
 * - Load() is the only operation that reports errors by **throwing**. Every
 *   failure raises an exception derived from core::ModuleException (see
 *   exception/module_exception/module_exception.md for the catalogue). It
 *   therefore never returns 0: the returned id is always valid.
 * - Every other operation is total and **never throws**. Lookups return
 *   nullptr when nothing matches, Unload() returns false when nothing was
 *   unloaded.
 *
 * ## Ownership and lifetimes
 *
 * The loader owns both the module instances and the shared libraries they come
 * from, and always destroys a module before closing its library.
 *
 * - Pointers returned by Module() stay valid until that module is unloaded
 *   (Unload(), UnloadAll()) or the loader is destroyed. They are never owning:
 *   callers must not delete them.
 * - Views returned by Modules() and Capabilities() are invalidated by **any**
 *   call to Load(), Unload() or UnloadAll(), by the next call to
 *   Capabilities() for the same capability type, and by the destruction of the
 *   loader. Copy what you need out of them instead of storing them.
 *
 * The loader is not thread safe; concurrent calls must be serialised by the
 * caller. It is neither copyable nor movable, because moving it would
 * invalidate every view it handed out.
 */
class ModuleLoader {
   public:
    ModuleLoader() = default;

    /**
     * @brief Unloads every module still loaded, in reverse load order.
     */
    ~ModuleLoader();

    ModuleLoader(const ModuleLoader &) = delete;
    ModuleLoader &operator=(const ModuleLoader &) = delete;
    ModuleLoader(ModuleLoader &&) = delete;
    ModuleLoader &operator=(ModuleLoader &&) = delete;

    /**
     * @brief Load a module from a shared library.
     *
     * The library is opened, its CreateModule factory is called, the resulting
     * module receives a unique nonzero id through IModule::Initialize() and is
     * checked for the mandatory Types() and Nodes() capabilities. Optional
     * capabilities are never required.
     *
     * Nothing is kept when the load fails: the module is shut down and
     * destroyed, the library is closed, the loader is left exactly as it was
     * and the rejected id is reused by the next call.
     *
     * @param path The path to the shared library. Required.
     *
     * @return The id of the loaded module. Always nonzero.
     *
     * @throws ModuleFileNotFoundException The path is empty, does not exist or
     * is not a regular file.
     * @throws ModuleLoadFailedException The operating system refused to load
     * the library.
     * @throws ModuleSymbolNotFoundException The library does not export
     * CreateModule.
     * @throws InvalidModuleException CreateModule returned nullptr, the module
     * has an empty name, it does not report the id it was given, or one of the
     * mandatory capabilities is missing.
     * @throws ModuleAlreadyLoadedException The same file, or another module
     * with the same name, is already loaded.
     * @throws ModuleInitializationException IModule::Initialize() returned
     * false.
     * @throws ModuleException The id space is exhausted.
     */
    ModuleId Load(std::filesystem::path path);

    /**
     * @brief Unload a module by its id.
     *
     * @param id The id of the module to unload.
     *
     * @return true if the module was successfully unloaded, false otherwise.
     */
    bool Unload(ModuleId id);

    /**
     * @brief Unload a module by its name.
     *
     * @param name The name of the module to unload, mendatory.
     *
     * @return true if the module was successfully unloaded,false otherwise.
     */
    bool Unload(std::string_view name);

    /**
     * @brief Unload all modules, in reverse load order.
     */
    void UnloadAll();

    /**
     * @brief Get a module by its id.
     *
     * @param id Id of the module, mendatory.
     *
     * @return A pointer to the requested module, nullptr if not found.
     */
    IModule *Module(ModuleId id) noexcept;

    /**
     * @brief Get a module by its name.
     *
     * @param name Name of the module, mendatory.
     *
     * @return A pointer to the requested module, nullptr if not found.
     */
    IModule *Module(std::string_view name) noexcept;

    /**
     * @brief Get a const module by its id.
     *
     * @param id Id of the module, mendatory.
     *
     * @return A const pointer to the requested module, nullptr if not found.
     */
    const IModule *Module(ModuleId id) const noexcept;

    /**
     * @brief Get a const module by its name.
     *
     * @param name Name of the module, mendatory.
     *
     * @return A const pointer to the requested module, nullptr if not found.
     */
    const IModule *Module(std::string_view name) const noexcept;

    /**
     * @brief Get a list of all modules, in load order.
     *
     * @return A list of pointers to all modules. The view is invalidated by
     * the next Load(), Unload() or UnloadAll() call.
     */
    std::span<const IModule *const> Modules() noexcept;

    /**
     * @brief This method retreives all capabilities of a certain type.
     * To get one from a specific module, ask the module itself.
     *
     * Modules that do not provide the capability are simply skipped, so an
     * empty span means no loaded module provides it.
     *
     * @return A list of pointers to all capabilities of the asked type. The
     * view is invalidated by the next call to Capabilities() for the same
     * capability type and by any Load(), Unload() or UnloadAll() call.
     */
    template <typename Capability>
    std::span<const Capability *> Capabilities();

    /**
     * @brief Number of currently loaded modules.
     *
     * @return The number of loaded modules.
     */
    std::size_t Size() const noexcept;

   private:
    /**
     * @brief One loaded module together with the library it came from.
     *
     * Declaration order is part of the contract: library_ is declared first so
     * that it is destroyed last, i.e. after the module instance whose code it
     * owns.
     */
    struct Entry {
        Entry() = default;
        ~Entry();

        Entry(const Entry &) = delete;
        Entry &operator=(const Entry &) = delete;
        Entry(Entry &&) = delete;
        Entry &operator=(Entry &&) = delete;

        detail::SharedLibrary library_;
        std::unique_ptr<IModule> instance_;
        std::filesystem::path path_;
        /// Shutdown() is only called on a module that reported a successful
        /// Initialize().
        bool initialized_ = false;
    };

    /**
     * @brief Find the entry owning the module with this id.
     *
     * @param id Id to look for.
     *
     * @return The owning entry, or nullptr when no module has this id.
     */
    const Entry *Find(ModuleId id) const noexcept;

    /**
     * @brief Find the entry owning the module with this name.
     *
     * @param name Name to look for.
     *
     * @return The owning entry, or nullptr when no module has this name.
     */
    const Entry *Find(std::string_view name) const noexcept;

    /**
     * @brief Rebuild the cache backing Modules() and drop stale capability
     * caches. Must be called after every mutation of entries_.
     */
    void RefreshCaches();

    /**
     * @brief Resolve one capability on one module.
     *
     * Uses IModule::Capability(), and falls back on the dedicated accessors
     * for the two mandatory capabilities so that a module does not have to
     * register them twice.
     *
     * @param instance Module to query. Required.
     *
     * @return The capability, or nullptr when the module does not provide it.
     */
    template <typename Capability>
    static const Capability *Resolve(IModule *instance) noexcept;

    std::vector<std::unique_ptr<Entry>> entries_;
    /// Mirror of entries_, so that Modules() can stay noexcept.
    std::vector<const IModule *> module_view_;
    /// Keeps the vector backing the last Capabilities<T>() span alive, one
    /// slot per capability type.
    std::unordered_map<std::type_index, std::shared_ptr<void>>
        capability_cache_;
    /// Ids are handed out monotonically and never reused, so that a stale id
    /// can never designate a different module.
    ModuleId next_id_ = 1;
};

}  // namespace core

// Retreive the implementation of the loader template functions
#include "loader.tcc"
