#pragma once

#include <filesystem>
#include <vector>

#include "module.hpp"

namespace core {

class ModuleLoader {
   public:
    std::pair<ModuleId, std::string_view> load(std::filesystem::path path);
    void unloadAll();


    IModule* module(ModuleId id) noexcept;
    IModule* module(std::string_view name) noexcept;
    const IModule* module(ModuleId id) const noexcept;
    const IModule* module(std::string_view name) const noexcept;
    std::span<const IModule*> modules() noexcept;

    /**
     * @brief This method retreives all capabilities of a certain type.
     * To get one from a specific module, ask the module itself.
     */
    template<typename Capability>
    std::vector<Capability*> capabilities();

   private:
    std::vector<std::unique_ptr<IModule>> _modules;
};

} // namespace core
