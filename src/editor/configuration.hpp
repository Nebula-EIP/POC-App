/**
 * @file Configuration.hpp
 * @brief Declaration of the Configuration class, which handles the loading
 *        and parsing of configuration files for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 12-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 19-11-2026
 */

#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string_view>

#include "exception/exception.hpp"

namespace editor {

/**
 * @brief Thrown when the configuration file cannot be read or is invalid
 *        (malformed YAML or JSON, value of the wrong type, empty value...).
 *
 * The message names the file and, for YAML, the line and column of the error.
 */
class ConfigurationException : public core::Exception {
   public:
    using core::Exception::Exception;
};

/**
 * @brief Represents the configuration for the editor, including paths to
 *        modules and other settings.
 *
 * Read once at startup from `config.yaml` (see `config.example.yaml`). The
 * file is optional, every key keeps its default value when it is absent, and
 * the application never writes it.
 *
 * Deprecated: when the YAML file is missing, the JSON file with the same name
 * (`config.json`) is read instead and a warning is logged. This fallback will
 * be removed.
 */
struct Configuration {
    static constexpr const char *kDefaultFile = "config.yaml";
    static constexpr const char *kDefaultModuleFolder = "modules";

    /// Folder the modules are loaded from. A relative value in the file is
    /// resolved from the folder containing the configuration file.
    std::filesystem::path module_folder_{kDefaultModuleFolder};

    /**
     * @brief Returns the configuration file selected by the command line.
     * @param arguments The command line arguments, without the program name.
     * @return kDefaultFile with no argument, `<path>` for `--config <path>`,
     *         std::nullopt for any other command line (usage error).
     */
    [[nodiscard]] static std::optional<std::filesystem::path> FileFromArguments(
        std::span<const std::string_view> arguments);

    /**
     * @brief Loads the configuration from @p config_file.
     *
     * A missing file gives the default configuration. A `.json` file (given
     * directly, or found next to a missing `.yaml` file) is read with the
     * deprecated JSON format.
     *
     * @throw ConfigurationException if the file cannot be read or is invalid.
     */
    [[nodiscard]] static Configuration Load(
        const std::filesystem::path &config_file = kDefaultFile);
};

}  // namespace editor
