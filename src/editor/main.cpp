/**
 * @file main.cpp
 * @brief Main entry point for the editor application.
 *
 * @author Created by Nathan Bezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 19-11-2026
 */

#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application.hpp"
#include "configuration.hpp"
#include "utils/palette.hpp"

namespace {
constexpr int kExitFailure = 1;
constexpr int kExitUsage = 2;
}  // namespace

int main(int argc, char **argv) {
    const std::vector<std::string_view> kArguments =
        argc > 1 ? std::vector<std::string_view>(argv + 1, argv + argc)
                 : std::vector<std::string_view>{};
    const std::optional<std::filesystem::path> kConfigFile =
        editor::Configuration::FileFromArguments(kArguments);
    if (!kConfigFile) {
        std::cerr << "usage: " << (argc > 0 ? argv[0] : "editor")
                  << " [--config <config-file>]\n";
        return kExitUsage;
    }

    try {
        const editor::Configuration kConfiguration =
            editor::Configuration::Load(*kConfigFile);
        std::vector<std::string> palette_warnings;
        palette::Set(
            palette::Palette::Load(palette::kDefaultFile, palette_warnings));
        for (const std::string &warning : palette_warnings) {
            std::cerr << "palette: " << warning << '\n';
        }
        editor::Application app{kConfiguration.module_folder_};
        return app.Run();
    } catch (const std::exception &error) {
        std::cerr << "fatal: " << error.what() << '\n';
        return kExitFailure;
    }
}
