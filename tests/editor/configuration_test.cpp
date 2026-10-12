/**
 * @file Configuration_test.cpp
 * @brief Implementation tests for the Configuration class, which handles the loading
 *        and parsing of configuration files for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 12-10-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 12-10-2026
 */

#include "configuration.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

class ConfigurationTest : public testing::Test {
   protected:
    void SetUp() override {
        directory_ = std::filesystem::temp_directory_path() /
                     "nebula_configuration_test";
        std::filesystem::create_directories(directory_);
    }

    void TearDown() override { std::filesystem::remove_all(directory_); }

    std::filesystem::path directory_;
};

TEST_F(ConfigurationTest, UsesDefaultsWhenFileIsMissing) {
    const editor::Configuration configuration =
        editor::Configuration::Load(directory_ / "missing.json");

    EXPECT_EQ(configuration.module_folder_, "modules");
}

TEST_F(ConfigurationTest, ResolvesRelativeFolderFromConfigurationDirectory) {
    const auto config_file = directory_ / "config.json";
    std::ofstream{config_file} << R"({"module_folder":"plugins"})";

    const editor::Configuration configuration =
        editor::Configuration::Load(config_file);

    EXPECT_EQ(configuration.module_folder_, directory_ / "plugins");
}

TEST_F(ConfigurationTest, RejectsInvalidFolderValue) {
    const auto config_file = directory_ / "config.json";
    std::ofstream{config_file} << R"({"module_folder":42})";

    EXPECT_THROW(static_cast<void>(editor::Configuration::Load(config_file)),
                 std::runtime_error);
}

}  // namespace
