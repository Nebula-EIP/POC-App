/**
 * @file loader_test.cpp
 * @brief Tests for core::ModuleLoader.
 *
 * Every test uses the shared library fixtures built by
 * tests/core/modules/fixtures. No window is ever opened.
 *
 * @author Created by ArthuryanLoheac
 * @date Created on 13-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 13-09-2026
 */

#include "modules/loader.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "exception/module_exception/module_load_exception.hpp"
#include "exception/module_exception/module_validation_exception.hpp"
#include "modules/capabilities/node_list_capability.hpp"
#include "modules/capabilities/renderer_capability.hpp"
#include "modules/capabilities/type_list_capability.hpp"

#ifdef NEBULA_FIXTURE_VALID

namespace {

/**
 * @brief Reads the lifecycle trace of a fixture library.
 *
 * The probe keeps its own handle on the library open, so the trace survives the
 * loader closing its handle and the statics of the library are shared with the
 * copy the loader opened.
 */
class FixtureProbe {
   public:
    /**
     * @brief Open a fixture library and reset its trace.
     *
     * @param path Path of the fixture library.
     */
    explicit FixtureProbe(const std::filesystem::path &path) {
        opened_ = library_.Open(path);
        events_ = reinterpret_cast<const char *(*)()>(
            library_.Symbol("FixtureEvents"));
        reset_ =
            reinterpret_cast<void (*)()>(library_.Symbol("FixtureResetEvents"));
        reset();
    }

    /// @return true when the library and both probe symbols were resolved.
    bool ok() const noexcept {
        return opened_ && events_ != nullptr && reset_ != nullptr;
    }

    /// @return The lifecycle trace accumulated so far.
    std::string events() const {
        return events_ == nullptr ? std::string{} : std::string{events_()};
    }

    /// @brief Clear the lifecycle trace.
    void reset() {
        if (reset_ != nullptr) {
            reset_();
        }
    }

   private:
    core::detail::SharedLibrary library_;
    bool opened_ = false;
    const char *(*events_)() = nullptr;
    void (*reset_)() = nullptr;
};

/**
 * @brief Common scratch directory for the tests that need real files.
 */
class ModuleLoaderTest : public ::testing::Test {
   protected:
    void SetUp() override {
        directory_ =
            std::filesystem::temp_directory_path() /
            ("nebula_loader_test_" +
             std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
             "_" + std::to_string(counter_++));
        std::filesystem::create_directories(directory_);
    }

    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(directory_, error);
    }

    /**
     * @brief Extension used by shared libraries on this platform.
     *
     * @return The extension, dot included.
     */
    static std::filesystem::path libraryExtension() {
        return std::filesystem::path(NEBULA_FIXTURE_VALID).extension();
    }

    /**
     * @brief Write a file that is not a shared library.
     *
     * @return Its path.
     */
    std::filesystem::path writeInvalidLibrary() const {
        std::filesystem::path path =
            directory_ / ("not_a_library" + libraryExtension().string());
        std::ofstream file(path, std::ios::binary);
        file << "This is definitely not a shared library.\n";
        file.close();
        return path;
    }

    /**
     * @brief Copy a fixture library under a new name.
     *
     * @param source Fixture to copy.
     * @param name File name of the copy, without extension.
     *
     * @return The path of the copy.
     */
    std::filesystem::path copyFixture(const std::filesystem::path &source,
                                      const std::string &name) const {
        std::filesystem::path path =
            directory_ / (name + libraryExtension().string());
        std::filesystem::copy_file(
            source, path, std::filesystem::copy_options::overwrite_existing);
        return path;
    }

    std::filesystem::path directory_;

   private:
    static int counter_;
};

int ModuleLoaderTest::counter_ = 0;

}  // namespace

// ============================================================================
// Successful load
// ============================================================================

TEST_F(ModuleLoaderTest, LoadsAValidModule) {
    core::ModuleLoader loader;

    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);

    EXPECT_NE(kId, 0U);
    EXPECT_EQ(loader.Size(), 1U);
    ASSERT_NE(loader.Module(kId), nullptr);
    EXPECT_EQ(loader.Module(kId)->Name(), "ValidFixture");
    EXPECT_EQ(loader.Module(kId)->Id(), kId);
}

TEST_F(ModuleLoaderTest, AssignsUniqueNonZeroIds) {
    core::ModuleLoader loader;

    const core::ModuleId kFirst = loader.Load(NEBULA_FIXTURE_VALID);
    const core::ModuleId kSecond = loader.Load(NEBULA_FIXTURE_SECOND);

    EXPECT_NE(kFirst, 0U);
    EXPECT_NE(kSecond, 0U);
    EXPECT_NE(kFirst, kSecond);
    EXPECT_EQ(loader.Size(), 2U);
}

TEST_F(ModuleLoaderTest, NeverReusesAnIdAfterUnload) {
    core::ModuleLoader loader;

    const core::ModuleId kFirst = loader.Load(NEBULA_FIXTURE_VALID);
    ASSERT_TRUE(loader.Unload(kFirst));
    const core::ModuleId kSecond = loader.Load(NEBULA_FIXTURE_VALID);

    EXPECT_NE(kFirst, kSecond);
    EXPECT_EQ(loader.Module(kFirst), nullptr);
    EXPECT_NE(loader.Module(kSecond), nullptr);
}

// ============================================================================
// Lookups
// ============================================================================

TEST_F(ModuleLoaderTest, FindsModulesByIdAndByName) {
    core::ModuleLoader loader;
    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);

    core::IModule *by_id = loader.Module(kId);
    core::IModule *by_name = loader.Module(std::string_view{"ValidFixture"});

    ASSERT_NE(by_id, nullptr);
    EXPECT_EQ(by_id, by_name);

    const core::ModuleLoader &const_loader = loader;
    EXPECT_EQ(const_loader.Module(kId), by_id);
    EXPECT_EQ(const_loader.Module(std::string_view{"ValidFixture"}), by_id);
}

TEST_F(ModuleLoaderTest, UnknownLookupsReturnNullptr) {
    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);

    EXPECT_EQ(loader.Module(static_cast<core::ModuleId>(4242)), nullptr);
    EXPECT_EQ(loader.Module(std::string_view{"NoSuchModule"}), nullptr);
    EXPECT_EQ(loader.Module(std::string_view{}), nullptr);

    const core::ModuleLoader &const_loader = loader;
    EXPECT_EQ(const_loader.Module(static_cast<core::ModuleId>(4242)), nullptr);
    EXPECT_EQ(const_loader.Module(std::string_view{"NoSuchModule"}), nullptr);
}

TEST_F(ModuleLoaderTest, ListsEveryModuleInLoadOrder) {
    core::ModuleLoader loader;
    const core::ModuleId kFirst = loader.Load(NEBULA_FIXTURE_VALID);
    const core::ModuleId kSecond = loader.Load(NEBULA_FIXTURE_SECOND);

    std::span<const core::IModule *const> modules = loader.Modules();

    ASSERT_EQ(modules.size(), 2U);
    EXPECT_EQ(modules[0], loader.Module(kFirst));
    EXPECT_EQ(modules[1], loader.Module(kSecond));
}

TEST_F(ModuleLoaderTest, ModulesViewShrinksAfterUnload) {
    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);
    loader.Load(NEBULA_FIXTURE_SECOND);

    ASSERT_TRUE(loader.Unload(std::string_view{"ValidFixture"}));

    ASSERT_EQ(loader.Modules().size(), 1U);
    EXPECT_EQ(loader.Modules()[0]->Name(), "SecondFixture");
}

// ============================================================================
// Capabilities
// ============================================================================

TEST_F(ModuleLoaderTest, ExposesMandatoryCapabilities) {
    core::ModuleLoader loader;
    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);

    core::IModule *module = loader.Module(kId);
    ASSERT_NE(module, nullptr);
    EXPECT_NE(module->Types(), nullptr);
    EXPECT_NE(module->Nodes(), nullptr);
}

TEST_F(ModuleLoaderTest, CollectsCapabilitiesAcrossModules) {
    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);
    loader.Load(NEBULA_FIXTURE_SECOND);

    EXPECT_EQ(loader.Capabilities<core::capa::ITypeListCapability>().size(),
              2U);
    EXPECT_EQ(loader.Capabilities<core::capa::INodeListCapability>().size(),
              2U);
}

TEST_F(ModuleLoaderTest, AcceptsModulesWithoutOptionalCapabilities) {
    core::ModuleLoader loader;
    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);

    core::IModule *module = loader.Module(kId);
    ASSERT_NE(module, nullptr);
    EXPECT_EQ(module->Capability<core::capa::IRendererCapability>(), nullptr);
    EXPECT_TRUE(loader.Capabilities<core::capa::IRendererCapability>().empty());
}

TEST_F(ModuleLoaderTest, ReportsOptionalCapabilitiesWhenProvided) {
    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);
    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_RENDERER);

    core::IModule *module = loader.Module(kId);
    ASSERT_NE(module, nullptr);
    EXPECT_NE(module->Capability<core::capa::IRendererCapability>(), nullptr);

    std::span<const core::capa::IRendererCapability *> renderers =
        loader.Capabilities<core::capa::IRendererCapability>();
    ASSERT_EQ(renderers.size(), 1U);
    EXPECT_EQ(renderers[0],
              module->Capability<core::capa::IRendererCapability>());
}

TEST_F(ModuleLoaderTest, CapabilityViewIsEmptyAfterUnloadAll) {
    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_RENDERER);
    ASSERT_EQ(loader.Capabilities<core::capa::IRendererCapability>().size(),
              1U);

    loader.UnloadAll();

    EXPECT_TRUE(loader.Capabilities<core::capa::IRendererCapability>().empty());
}

// ============================================================================
// Failed loads
// ============================================================================

TEST_F(ModuleLoaderTest, RejectsAMissingFile) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(directory_ / "does_not_exist.so"),
                 core::ModuleFileNotFoundException);
    EXPECT_THROW(loader.Load(std::filesystem::path{}),
                 core::ModuleFileNotFoundException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsADirectory) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(directory_), core::ModuleFileNotFoundException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsAnInvalidLibrary) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(writeInvalidLibrary()),
                 core::ModuleLoadFailedException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsALibraryWithoutFactory) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_NO_FACTORY),
                 core::ModuleSymbolNotFoundException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsAFactoryReturningNullptr) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_NULL_FACTORY),
                 core::InvalidModuleException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsAModuleThatFailsToInitialize) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_INIT_FAIL),
                 core::ModuleInitializationException);
    EXPECT_EQ(loader.Size(), 0U);
    EXPECT_EQ(loader.Module(std::string_view{"InitFailFixture"}), nullptr);
}

TEST_F(ModuleLoaderTest, RejectsAModuleWithoutTypeListCapability) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_NO_TYPES),
                 core::InvalidModuleException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsAModuleWithoutNodeListCapability) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_NO_NODES),
                 core::InvalidModuleException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsAModuleReportingTheWrongId) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_WRONG_ID),
                 core::InvalidModuleException);
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, RejectsTheSameLibraryTwice) {
    core::ModuleLoader loader;
    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);

    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_VALID),
                 core::ModuleAlreadyLoadedException);
    EXPECT_EQ(loader.Size(), 1U);
    EXPECT_NE(loader.Module(kId), nullptr);
}

TEST_F(ModuleLoaderTest, RejectsTwoModulesSharingAName) {
    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);
    const std::filesystem::path kCopy =
        copyFixture(NEBULA_FIXTURE_VALID, "valid_fixture_copy");

    EXPECT_THROW(loader.Load(kCopy), core::ModuleAlreadyLoadedException);
    EXPECT_EQ(loader.Size(), 1U);
}

// ============================================================================
// Recovery and cleanup
// ============================================================================

TEST_F(ModuleLoaderTest, RecoversFromFailedLoads) {
    core::ModuleLoader loader;

    EXPECT_THROW(loader.Load(directory_ / "missing.so"),
                 core::ModuleFileNotFoundException);
    EXPECT_THROW(loader.Load(writeInvalidLibrary()),
                 core::ModuleLoadFailedException);
    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_NO_FACTORY),
                 core::ModuleSymbolNotFoundException);
    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_NULL_FACTORY),
                 core::InvalidModuleException);
    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_INIT_FAIL),
                 core::ModuleInitializationException);

    ASSERT_EQ(loader.Size(), 0U);
    EXPECT_TRUE(loader.Modules().empty());

    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);
    EXPECT_NE(kId, 0U);
    EXPECT_EQ(loader.Size(), 1U);
}

TEST_F(ModuleLoaderTest, AFailedLoadDestroysWhatItCreated) {
    FixtureProbe probe(NEBULA_FIXTURE_INIT_FAIL);
    ASSERT_TRUE(probe.ok());

    core::ModuleLoader loader;
    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_INIT_FAIL),
                 core::ModuleInitializationException);

    // Constructed, then destroyed. Shutdown() is not called on a module whose
    // Initialize() failed.
    EXPECT_EQ(probe.events(), "CD");
}

TEST_F(ModuleLoaderTest, AFailedValidationShutsTheModuleDownBeforeDestroying) {
    FixtureProbe probe(NEBULA_FIXTURE_NO_TYPES);
    ASSERT_TRUE(probe.ok());

    core::ModuleLoader loader;
    EXPECT_THROW(loader.Load(NEBULA_FIXTURE_NO_TYPES),
                 core::InvalidModuleException);

    EXPECT_EQ(probe.events(), "CSD");
}

TEST_F(ModuleLoaderTest, UnloadByIdShutsDownThenDestroys) {
    FixtureProbe probe(NEBULA_FIXTURE_VALID);
    ASSERT_TRUE(probe.ok());

    core::ModuleLoader loader;
    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);
    ASSERT_EQ(probe.events(), "C");

    EXPECT_TRUE(loader.Unload(kId));

    EXPECT_EQ(probe.events(), "CSD");
    EXPECT_EQ(loader.Size(), 0U);
    EXPECT_EQ(loader.Module(kId), nullptr);
    EXPECT_FALSE(loader.Unload(kId));
}

TEST_F(ModuleLoaderTest, UnloadByNameShutsDownThenDestroys) {
    FixtureProbe probe(NEBULA_FIXTURE_VALID);
    ASSERT_TRUE(probe.ok());

    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);

    EXPECT_TRUE(loader.Unload(std::string_view{"ValidFixture"}));

    EXPECT_EQ(probe.events(), "CSD");
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, UnloadingAnUnknownModuleReturnsFalse) {
    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);

    EXPECT_FALSE(loader.Unload(static_cast<core::ModuleId>(999)));
    EXPECT_FALSE(loader.Unload(std::string_view{"NoSuchModule"}));
    EXPECT_FALSE(loader.Unload(std::string_view{}));
    EXPECT_EQ(loader.Size(), 1U);
}

TEST_F(ModuleLoaderTest, UnloadAllCleansUpEveryModule) {
    FixtureProbe valid(NEBULA_FIXTURE_VALID);
    FixtureProbe second(NEBULA_FIXTURE_SECOND);
    FixtureProbe renderer(NEBULA_FIXTURE_RENDERER);
    ASSERT_TRUE(valid.ok());
    ASSERT_TRUE(second.ok());
    ASSERT_TRUE(renderer.ok());

    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);
    loader.Load(NEBULA_FIXTURE_SECOND);
    loader.Load(NEBULA_FIXTURE_RENDERER);
    ASSERT_EQ(loader.Size(), 3U);

    loader.UnloadAll();

    EXPECT_EQ(loader.Size(), 0U);
    EXPECT_TRUE(loader.Modules().empty());
    EXPECT_EQ(valid.events(), "CSD");
    EXPECT_EQ(second.events(), "CSD");
    EXPECT_EQ(renderer.events(), "CSD");

    // Calling it again is harmless.
    loader.UnloadAll();
    EXPECT_EQ(loader.Size(), 0U);
}

TEST_F(ModuleLoaderTest, DestructionCleansUpEveryModule) {
    FixtureProbe valid(NEBULA_FIXTURE_VALID);
    FixtureProbe second(NEBULA_FIXTURE_SECOND);
    ASSERT_TRUE(valid.ok());
    ASSERT_TRUE(second.ok());

    {
        core::ModuleLoader loader;
        loader.Load(NEBULA_FIXTURE_VALID);
        loader.Load(NEBULA_FIXTURE_SECOND);
        ASSERT_EQ(loader.Size(), 2U);
    }

    EXPECT_EQ(valid.events(), "CSD");
    EXPECT_EQ(second.events(), "CSD");
}

TEST_F(ModuleLoaderTest, UnloadingOneModuleLeavesTheOthersUntouched) {
    FixtureProbe valid(NEBULA_FIXTURE_VALID);
    FixtureProbe second(NEBULA_FIXTURE_SECOND);
    ASSERT_TRUE(valid.ok());
    ASSERT_TRUE(second.ok());

    core::ModuleLoader loader;
    loader.Load(NEBULA_FIXTURE_VALID);
    loader.Load(NEBULA_FIXTURE_SECOND);

    EXPECT_TRUE(loader.Unload(std::string_view{"SecondFixture"}));
    EXPECT_EQ(second.events(), "CSD");
    EXPECT_EQ(valid.events(), "C");

    loader.UnloadAll();
    EXPECT_EQ(valid.events(), "CSD");
}

TEST_F(ModuleLoaderTest, AModuleStaysUsableUntilItIsUnloaded) {
    core::ModuleLoader loader;
    const core::ModuleId kId = loader.Load(NEBULA_FIXTURE_VALID);

    core::IModule *module = loader.Module(kId);
    ASSERT_NE(module, nullptr);

    core::capa::ITypeListCapability *types = module->Types();
    ASSERT_NE(types, nullptr);
    const std::string_view *registered = types->RegisterType(7);
    ASSERT_NE(registered, nullptr);
    EXPECT_EQ(*registered, "FixtureType");
    EXPECT_EQ(types->TypeId("FixtureType"), 7U);
    EXPECT_EQ(types->TypeName(7), "FixtureType");

    core::capa::INodeListCapability *nodes = module->Nodes();
    ASSERT_NE(nodes, nullptr);
    const std::string_view *node = nodes->RegisterNode(3);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(*node, "FixtureNode");
    ASSERT_EQ(nodes->GetAvailableNodes().size(), 1U);
    EXPECT_EQ(nodes->GetAvailableNodes()[0].name_, "FixtureNode");
}

#endif  // NEBULA_FIXTURE_VALID
