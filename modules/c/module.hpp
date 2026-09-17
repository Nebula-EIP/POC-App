/**
 * @file module.hpp
 * @brief Module class for the C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 14-09-2026
 */

#pragma once

#include "modules/module.hpp"
#include "modules/capabilities/renderer_capability.hpp"
#include "modules/capabilities/importer_capability.hpp"
#include "modules/capabilities/exporter_capability.hpp"

/**
 * @brief C Module class
 */
class CModule : public core::IModule {
   public:
    CModule();
    ~CModule() = default;

    /**
     * @brief Get the name of the module.
     *
     * @return The name of the module.
     */
    virtual std::string_view Name() const noexcept override;

    /**
     * @brief Get the author of the module.
     *
     * @return The author of the module.
     */
    virtual std::string_view Author() const noexcept override;

    /**
     * @brief Get the description of the module.
     *
     * @return The description of the module.
     */
    virtual std::string_view Description() const noexcept override;

    /**
     * @brief Get the version of the module.
     *
     * @return The version of the module.
     */
    virtual core::IModule::Version GetVersion() const noexcept override;

    /**
     * @brief Initialize the module
     *
     * @param id The id of the module, assigned by the core. Required.
     *
     * @return true if the module has been initialized, false if not
     */
    virtual bool Initialize(core::ModuleId id) override;

    /**
     * @brief Ask the module to release all resources and prepare for shutdown.
     */
    virtual void Shutdown() noexcept override;

    /**
     * @brief Get the id of the module.
     *
     * @return The id of the module.
     */
    virtual core::ModuleId Id() const noexcept;

    /**
     * @brief Get a capability by type.
     *
     * @param T The type of the capability to get.
     *
     * @return A pointer to the capability, or nullptr if not found.
     */
    virtual core::capa::ITypeListCapability *Types() override;

    /**
     * @brief Get a const capability by type.
     *
     * @param T The type of the capability to get.
     *
     * @return A const pointer to the capability, or nullptr if not found.
     */
    virtual core::capa::INodeListCapability *Nodes() override;

   private:
    virtual core::ICapability *Capability(std::type_index type) noexcept override;

    virtual const core::ICapability *Capability(std::type_index) const noexcept override;

    core::ModuleId id_;

    std::unique_ptr<core::capa::ITypeListCapability> types_;
    std::unique_ptr<core::capa::INodeListCapability> nodes_;
    std::unique_ptr<core::capa::IRendererCapability> renderer_;
    std::unique_ptr<core::capa::IImporterCapability> importer_;
    std::unique_ptr<core::capa::IExporterCapability> exporter_;
};
