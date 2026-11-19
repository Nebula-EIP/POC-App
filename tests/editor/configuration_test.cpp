/**
 * @file Configuration_test.cpp
 * @brief Implementation tests for the Configuration class, which handles the
 * loading and parsing of configuration files for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 12-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 19-11-2026
 */

#include "configuration.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

namespace {

using testing::HasSubstr;
using testing::Not;

class ConfigurationTest : public testing::Test {
   protected:
    void SetUp() override {
        const auto *test_info =
            testing::UnitTest::GetInstance()->current_test_info();
        directory_ =
            std::filesystem::temp_directory_path() /
            ("nebula_configuration_test_" + std::string{test_info->name()});
        std::filesystem::remove_all(directory_);
        std::filesystem::create_directories(directory_);
    }

    void TearDown() override { std::filesystem::remove_all(directory_); }

    /// Writes @p contents to @p name in the test directory.
    std::filesystem::path Write(const std::string &name,
                                const std::string &contents) const {
        const auto kFile = directory_ / name;
        std::ofstream{kFile} << contents;
        return kFile;
    }

    /// Message of the ConfigurationException thrown by loading @p file.
    static std::string LoadError(const std::filesystem::path &file) {
        try {
            static_cast<void>(editor::Configuration::Load(file));
        } catch (const editor::ConfigurationException &error) {
            return error.what();
        }
        ADD_FAILURE() << "no ConfigurationException for " << file;
        return {};
    }

    std::filesystem::path directory_;
};

// --- Missing file -----------------------------------------------------------

TEST_F(ConfigurationTest, UsesDefaultsWhenFileIsMissing) {
    const editor::Configuration kConfiguration =
        editor::Configuration::Load(directory_ / "config.yaml");

    EXPECT_EQ(kConfiguration.module_folder_, "modules");
}

TEST_F(ConfigurationTest, DefaultFileIsConfigYaml) {
    EXPECT_STREQ(editor::Configuration::kDefaultFile, "config.yaml");
}

// --- Valid file -------------------------------------------------------------

TEST_F(ConfigurationTest, ReadsAbsoluteModuleFolder) {
    const auto kFolder = directory_ / "absolute" / "modules";
    const auto kFile =
        Write("config.yaml",
              "# Nebula\nmodule_folder: '" + kFolder.generic_string() + "'\n");

    const editor::Configuration kConfiguration =
        editor::Configuration::Load(kFile);

    EXPECT_EQ(kConfiguration.module_folder_, kFolder);
}

TEST_F(ConfigurationTest, UsesDefaultsForEmptyFile) {
    const auto kFile = Write("config.yaml", "# Only a comment\n");

    EXPECT_EQ(editor::Configuration::Load(kFile).module_folder_, "modules");
}

TEST_F(ConfigurationTest, IgnoresUnknownKeyWithWarning) {
    const auto kFile =
        Write("config.yaml", "language: fr\nmodule_folder: plugins\n");

    testing::internal::CaptureStdout();
    const editor::Configuration kConfiguration =
        editor::Configuration::Load(kFile);
    const std::string kLog = testing::internal::GetCapturedStdout();

    EXPECT_EQ(kConfiguration.module_folder_, directory_ / "plugins");
    EXPECT_THAT(kLog, HasSubstr("Unknown key 'language'"));
}

// --- Invalid file -----------------------------------------------------------

TEST_F(ConfigurationTest, RejectsInvalidYamlWithPosition) {
    const auto kFile = Write("config.yaml", "module_folder: [plugins\n");

    const std::string kError = LoadError(kFile);

    EXPECT_THAT(kError, HasSubstr(kFile.string()));
    EXPECT_THAT(kError, HasSubstr("line "));
}

TEST_F(ConfigurationTest, InvalidYamlIsARuntimeError) {
    const auto kFile = Write("config.yaml", "module_folder: [plugins\n");

    EXPECT_THROW(static_cast<void>(editor::Configuration::Load(kFile)),
                 std::runtime_error);
}

TEST_F(ConfigurationTest, RejectsNonStringFolder) {
    const auto kFile = Write("config.yaml", "module_folder:\n  - a\n  - b\n");

    EXPECT_THAT(LoadError(kFile),
                HasSubstr("'module_folder' must be a string (line 2"));
}

TEST_F(ConfigurationTest, RejectsMissingFolderValue) {
    const auto kFile = Write("config.yaml", "module_folder:\n");

    EXPECT_THAT(LoadError(kFile),
                HasSubstr("'module_folder' must be a string"));
}

TEST_F(ConfigurationTest, RejectsEmptyFolder) {
    const auto kFile = Write("config.yaml", "module_folder: \"\"\n");

    EXPECT_THAT(LoadError(kFile),
                HasSubstr("'module_folder' must not be empty"));
}

TEST_F(ConfigurationTest, RejectsRootThatIsNotAMapping) {
    const auto kFile = Write("config.yaml", "- modules\n");

    EXPECT_THAT(LoadError(kFile), HasSubstr("expected 'key: value' entries"));
}

TEST_F(ConfigurationTest, RejectsFolderAsConfigurationFile) {
    EXPECT_THROW(static_cast<void>(editor::Configuration::Load(directory_)),
                 editor::ConfigurationException);
}

// --- Relative path ----------------------------------------------------------

TEST_F(ConfigurationTest, ResolvesRelativeFolderFromConfigurationDirectory) {
    const auto kFile = Write("config.yaml", "module_folder: plugins\n");

    const editor::Configuration kConfiguration =
        editor::Configuration::Load(kFile);

    EXPECT_EQ(kConfiguration.module_folder_, directory_ / "plugins");
}

TEST_F(ConfigurationTest, ResolvesNestedRelativeFolder) {
    const auto kFile =
        Write("config.yaml", "module_folder: build/manual-modules\n");

    EXPECT_EQ(editor::Configuration::Load(kFile).module_folder_,
              directory_ / "build/manual-modules");
}

// --- --config ---------------------------------------------------------------

TEST_F(ConfigurationTest, UsesDefaultFileWithoutArguments) {
    EXPECT_EQ(editor::Configuration::FileFromArguments({}),
              std::filesystem::path{"config.yaml"});
}

TEST_F(ConfigurationTest, ConfigOptionSelectsFile) {
    std::filesystem::create_directories(directory_ / "custom");
    const auto kFile = Write("custom/editor.yaml", "module_folder: plugins\n");
    const std::string kPath = kFile.string();
    const std::array<std::string_view, 2> kArguments{"--config", kPath};

    const std::optional<std::filesystem::path> kSelected =
        editor::Configuration::FileFromArguments(kArguments);

    ASSERT_EQ(kSelected, kFile);
    EXPECT_EQ(editor::Configuration::Load(*kSelected).module_folder_,
              directory_ / "custom" / "plugins");
}

TEST_F(ConfigurationTest, RejectsInvalidArguments) {
    const std::array<std::string_view, 1> kMissingPath{"--config"};
    const std::array<std::string_view, 2> kUnknownOption{"--conf", "a.yaml"};
    const std::array<std::string_view, 3> kTooMany{"--config", "a", "b"};

    EXPECT_FALSE(editor::Configuration::FileFromArguments(kMissingPath));
    EXPECT_FALSE(editor::Configuration::FileFromArguments(kUnknownOption));
    EXPECT_FALSE(editor::Configuration::FileFromArguments(kTooMany));
}

// --- Deprecated config.json -------------------------------------------------

TEST_F(ConfigurationTest, FallsBackToLegacyJsonWithWarning) {
    Write("config.json", R"({"module_folder":"plugins"})");

    testing::internal::CaptureStdout();
    const editor::Configuration kConfiguration =
        editor::Configuration::Load(directory_ / "config.yaml");
    const std::string kLog = testing::internal::GetCapturedStdout();

    EXPECT_EQ(kConfiguration.module_folder_, directory_ / "plugins");
    EXPECT_THAT(kLog, HasSubstr("deprecated JSON format"));
}

TEST_F(ConfigurationTest, PrefersYamlOverLegacyJson) {
    Write("config.json", R"({"module_folder":"from-json"})");
    const auto kFile = Write("config.yaml", "module_folder: from-yaml\n");

    testing::internal::CaptureStdout();
    const editor::Configuration kConfiguration =
        editor::Configuration::Load(kFile);
    const std::string kLog = testing::internal::GetCapturedStdout();

    EXPECT_EQ(kConfiguration.module_folder_, directory_ / "from-yaml");
    EXPECT_THAT(kLog, Not(HasSubstr("deprecated")));
}

TEST_F(ConfigurationTest, RejectsInvalidLegacyJson) {
    const auto kNumber = Write("number.json", R"({"module_folder":42})");
    const auto kUnknown = Write("unknown.json", R"({"modules":"plugins"})");
    const auto kSyntax = Write("syntax.json", R"({"module_folder":)");

    testing::internal::CaptureStdout();
    EXPECT_THAT(LoadError(kNumber), HasSubstr("must be a non-empty string"));
    EXPECT_THAT(LoadError(kUnknown), HasSubstr("unknown key 'modules'"));
    EXPECT_THAT(LoadError(kSyntax), HasSubstr(kSyntax.string()));
    static_cast<void>(testing::internal::GetCapturedStdout());
}

}  // namespace
