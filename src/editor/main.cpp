/**
 * @file main.cpp
 * @brief Main entry point for the editor application.
 *
 * @author Created by Nathan Bezard
 * @date Created on 19-09-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 12-10-2026
 */

#include <exception>
#include <iostream>

#include "application.hpp"
#include "configuration.hpp"

namespace {
constexpr int kExitFailure = 1;
constexpr int kExitUsage = 2;
}  // namespace

int main(int argc, char **argv) {
    if (argc != 1 && !(argc == 3 && std::string{argv[1]} == "--config")) {
        std::cerr << "usage: " << (argc > 0 ? argv[0] : "editor")
                  << " [--config <config-file>]\n";
        return kExitUsage;
    }

    try {
        const std::filesystem::path kConfigFile =
            argc == 3 ? argv[2] : editor::Configuration::kDefaultFile;
        const editor::Configuration kConfiguration =
            editor::Configuration::Load(kConfigFile);
        editor::Application app{kConfiguration.module_folder};
        return app.Run();
    } catch (const std::exception &error) {
        std::cerr << "fatal: " << error.what() << '\n';
        return kExitFailure;
    }
}
