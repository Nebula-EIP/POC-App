#pragma once

#include <filesystem>
#include <vector>

#include "module.hpp"

namespace core {

class ModuleLoader {
   public:
    /**
     * @returns The id given to the module, if 0 it means an error has occured.
     */
    ModuleId load(std::filesystem::path path);
    bool unload(ModuleId);
    bool unload(std::string_view name);
    void unloadAll();


    IModule* module(ModuleId id) noexcept;
    IModule* module(std::string_view name) noexcept;
    const IModule* module(ModuleId id) const noexcept;
    const IModule* module(std::string_view name) const noexcept;
    std::span<const IModule* const> modules() noexcept;

    /**
     * @brief This method retreives all capabilities of a certain type.
     * To get one from a specific module, ask the module itself.
     */
    template<typename Capability>
    std::vector<const Capability*> capabilities();

   private:
    std::vector<std::unique_ptr<IModule>> _modules;
};

} // namespace core
