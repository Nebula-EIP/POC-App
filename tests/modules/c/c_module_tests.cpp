/**
 * @file c_module_tests.cpp
 * @brief Comprehensive tests for the CModule class
 *
 * @author Created by JeanBizeul
 * @date Created on 14-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 20-09-2026
 */

#include <filesystem>
#include <string>

#include "gtest/gtest.h"
#include "modules/capabilities/exporter_capability.hpp"
#include "modules/capabilities/importer_capability.hpp"
#include "modules/capabilities/renderer_capability.hpp"
#include "modules/loader.hpp"

namespace {

class CModuleTest : public testing::Test {
   protected:
    void SetUp() override { loader_ = std::make_unique<core::ModuleLoader>(); };

    void TearDown() override { loader_->UnloadAll(); }

    static std::filesystem::path ModulePath() {
#ifdef NEBULA_C_MODULE
        return std::filesystem::path(NEBULA_C_MODULE);
#else
#error "Cannot find the C Module"
#endif
    }

    core::ModuleLoader &Loader() { return *loader_; }

    core::IModule *LoadCModule() {
        const auto id = loader_->Load(ModulePath());

        return loader_->Module(id);
    }

    std::unique_ptr<core::ModuleLoader> loader_;
};

/**
 * The shared library exists at the expected location.
 */
TEST_F(CModuleTest, LibraryExists) {
    const auto path = ModulePath();

    ASSERT_TRUE(std::filesystem::exists(path))
        << "C module does not exist: " << path;

    EXPECT_TRUE(std::filesystem::is_regular_file(path))
        << "C module path is not a regular file: " << path;
}

/**
 * The C hook can be resolved and the module can be loaded successfully.
 */
TEST_F(CModuleTest, CanLoadModule) {
    const auto id = Loader().Load(ModulePath());

    EXPECT_NE(id, core::ModuleId{0});
    EXPECT_EQ(Loader().Size(), 1u);
}

/**
 * Loading the module produces a valid IModule instance.
 */
TEST_F(CModuleTest, LoadedModuleIsNotNull) {
    const auto id = Loader().Load(ModulePath());

    core::IModule *module = Loader().Module(id);

    ASSERT_NE(module, nullptr);
}

/**
 * The module can be retrieved by its assigned ID.
 */
TEST_F(CModuleTest, CanRetrieveModuleById) {
    const auto id = Loader().Load(ModulePath());

    core::IModule *module = Loader().Module(id);

    ASSERT_NE(module, nullptr);
    EXPECT_EQ(module->Id(), id);
}

/**
 * The module can be retrieved by its name.
 */
TEST_F(CModuleTest, HasValidName) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);
    EXPECT_FALSE(module->Name().empty());
}

/**
 * The author field is available.
 */
TEST_F(CModuleTest, HasAuthor) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);
    EXPECT_FALSE(module->Author().empty());
}

/**
 * The description field is available.
 */
TEST_F(CModuleTest, HasDescription) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);
    EXPECT_FALSE(module->Description().empty());
}

/**
 * The module exposes a valid version.
 */
TEST_F(CModuleTest, HasValidVersion) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    const auto version = module->GetVersion();

    // Version components are unsigned 16-bit values, but explicitly checking
    // that GetVersion() can be called and returns a coherent object is useful.
    EXPECT_LE(version.major_, std::numeric_limits<uint16_t>::max());
    EXPECT_LE(version.minor_, std::numeric_limits<uint16_t>::max());
    EXPECT_LE(version.patch_, std::numeric_limits<uint16_t>::max());
}

/**
 * The mandatory type-list capability is available.
 */
TEST_F(CModuleTest, HasTypeListCapability) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    EXPECT_NE(module->Types(), nullptr);
}

/**
 * The mandatory node-list capability is available.
 */
TEST_F(CModuleTest, HasNodeListCapability) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    EXPECT_NE(module->Nodes(), nullptr);
}

/**
 * The renderer capability is available when CModule provides it.
 *
 */
TEST_F(CModuleTest, HasRendererCapability) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    const auto *renderer =
        module->Capability<core::capa::IRendererCapability>();
    ASSERT_NE(renderer, nullptr);
    EXPECT_TRUE(renderer->SupportsNodeType(1));
}

/**
 * The importer capability is available when CModule provides it.
 *
 * Temporary null while the capability is not yet implemented
 */
TEST_F(CModuleTest, HasImporterCapability) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    EXPECT_EQ(module->Capability<core::capa::IImporterCapability>(), nullptr);
}

/**
 * The exporter capability is available when CModule provides it.
 *
 * Temporary null while the capability is not yet implemented
 */
TEST_F(CModuleTest, HasExporterCapability) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    EXPECT_EQ(module->Capability<core::capa::IExporterCapability>(), nullptr);
}

/**
 * The mandatory capabilities can also be retrieved through the generic
 * Capability<T>() interface.
 */
TEST_F(CModuleTest, MandatoryCapabilitiesAreRegistered) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    EXPECT_NE(module->Capability<core::capa::ITypeListCapability>(), nullptr);

    EXPECT_NE(module->Capability<core::capa::INodeListCapability>(), nullptr);
}

/**
 * The module can be unloaded by ID.
 */
TEST_F(CModuleTest, CanUnloadById) {
    const auto id = Loader().Load(ModulePath());

    ASSERT_EQ(Loader().Size(), 1u);
    ASSERT_NE(Loader().Module(id), nullptr);

    EXPECT_TRUE(Loader().Unload(id));

    EXPECT_EQ(Loader().Size(), 0u);
    EXPECT_EQ(Loader().Module(id), nullptr);
}

/**
 * The module can be unloaded by name.
 */
TEST_F(CModuleTest, CanUnloadByName) {
    core::IModule *module = LoadCModule();

    ASSERT_NE(module, nullptr);

    const std::string name{module->Name()};

    EXPECT_TRUE(Loader().Unload(name));

    EXPECT_EQ(Loader().Size(), 0u);
    EXPECT_EQ(Loader().Module(name), nullptr);
}

/**
 * The C module can be loaded, unloaded, and loaded again.
 */
TEST_F(CModuleTest, CanReloadModule) {
    const auto first_id = Loader().Load(ModulePath());

    ASSERT_NE(Loader().Module(first_id), nullptr);
    ASSERT_TRUE(Loader().Unload(first_id));
    ASSERT_EQ(Loader().Size(), 0u);

    const auto second_id = Loader().Load(ModulePath());

    ASSERT_NE(second_id, core::ModuleId{0});
    EXPECT_EQ(Loader().Size(), 1u);
    EXPECT_NE(Loader().Module(second_id), nullptr);
}

}  // namespace
