/**
 * @file Configuration.hpp
 * @brief Declaration of the Configuration class, which handles the loading
 *        and parsing of configuration files for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 12-10-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 12-10-2026
 */

#pragma once

#include <filesystem>

namespace editor {

/**
 * @brief Represents the configuration for the editor, including paths to
 *        modules and other settings.
 */
struct Configuration {
    static constexpr const char *kDefaultFile = "config.json";
    static constexpr const char *kDefaultModuleFolder = "modules";

    std::filesystem::path module_folder_;

    [[nodiscard]] static Configuration Load(
        const std::filesystem::path &config_file = kDefaultFile);
};

}  // namespace editor
