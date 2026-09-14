/**
 * @file module.cpp
 * @brief Hook used to retreive the C module
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by JeanBizeul
 * @date Last modified on 14-09-2026
 */

#include "module.hpp"

#include "modules_inits.hpp"

constexpr std::string_view kName = "C";
constexpr std::string_view kAuthor = "Nebula Team";
constexpr std::string_view kDescription = "C language module - full suite";

constexpr core::IModule::Version kVersion = {
    .major_ = 1,
    .minor_ = 0,
    .patch_ = 0
};

CModule::CModule()
    : types_(nullptr), nodes_(nullptr), renderer_(nullptr), importer_(nullptr), exporter_(nullptr)
{}

std::string_view CModule::Name() const noexcept
{
    return kName;
}

std::string_view CModule::Author() const noexcept
{
    return kAuthor;
}

std::string_view CModule::Description() const noexcept
{
    return kDescription;
}

core::IModule::Version CModule::GetVersion() const noexcept
{
    return kVersion;
}

bool CModule::Initialize(core::ModuleId id)
{
    id_ = id;

    types_.reset(CreateTypeListCapa());
    nodes_.reset(CreateNodeListCapa());
    /// Initialize others capabilities here.
    return true;
}

void CModule::Shutdown() noexcept
{
    types_.reset();
    nodes_.reset();
    renderer_.reset();
    importer_.reset();
    exporter_.reset();
}

core::ModuleId CModule::Id() const noexcept
{
    return id_;
}

core::capa::ITypeListCapability *CModule::Types()
{
    return types_.get();
}

core::capa::INodeListCapability *CModule::Nodes()
{
    return nodes_.get();
}

core::ICapability *CModule::Capability(std::type_index type) noexcept
{
    if (type == std::type_index(typeid(core::capa::ITypeListCapability))) {
        return types_.get();
    } else if (type == std::type_index(typeid(core::capa::INodeListCapability))) {
        return nodes_.get();
    } else if (type == std::type_index(typeid(core::capa::IRendererCapability))) {
        return renderer_.get();
    } else if (type == std::type_index(typeid(core::capa::IImporterCapability))) {
        return importer_.get();
    } else if (type == std::type_index(typeid(core::capa::IExporterCapability))) {
        return exporter_.get();
    } else {
        return nullptr;
    }
}

const core::ICapability *CModule::Capability(std::type_index type) const noexcept
{
    if (type == std::type_index(typeid(core::capa::ITypeListCapability))) {
        return types_.get();
    } else if (type == std::type_index(typeid(core::capa::INodeListCapability))) {
        return nodes_.get();
    } else if (type == std::type_index(typeid(core::capa::IRendererCapability))) {
        return renderer_.get();
    } else if (type == std::type_index(typeid(core::capa::IImporterCapability))) {
        return importer_.get();
    } else if (type == std::type_index(typeid(core::capa::IExporterCapability))) {
        return exporter_.get();
    } else {
        return nullptr;
    }
}
