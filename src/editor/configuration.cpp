/**
 * @file Configuration.cpp
 * @brief Implementation of the Configuration class, which handles the loading
 *        and parsing of configuration files for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 12-10-2026
 *
 * @author Last modified by ArthuryanLoheac
 * @date Last modified on 19-11-2026
 */

#include "configuration.hpp"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <string>
#include <system_error>
#include <utility>

#include "utils/logger.hpp"

namespace editor {

namespace {

constexpr std::string_view kModuleFolderKey = "module_folder";

[[noreturn]] void Fail(const std::filesystem::path &config_file,
                       const std::string &message) {
    throw ConfigurationException("Invalid configuration file '" +
                                 config_file.string() + "': " + message);
}

/// " (line L, column C)" for a YAML position, empty when it is unknown.
std::string Where(const YAML::Mark &mark) {
    if (mark.is_null()) {
        return {};
    }
    return " (line " + std::to_string(mark.line + 1) + ", column " +
           std::to_string(mark.column + 1) + ")";
}

/// Returns @p value as a non-empty string. Any YAML scalar is accepted.
std::string ReadString(const YAML::Node &value, std::string_view key,
                       const std::filesystem::path &config_file) {
    if (!value.IsScalar()) {
        Fail(config_file, "'" + std::string{key} + "' must be a string" +
                              Where(value.Mark()));
    }
    if (value.Scalar().empty()) {
        Fail(config_file, "'" + std::string{key} + "' must not be empty" +
                              Where(value.Mark()));
    }
    return value.Scalar();
}

/// Relative folders are resolved from the folder of the configuration file.
void SetModuleFolder(Configuration &configuration, std::string folder,
                     const std::filesystem::path &config_file) {
    configuration.module_folder_ = std::move(folder);
    if (configuration.module_folder_.is_relative()) {
        configuration.module_folder_ =
            config_file.parent_path() / configuration.module_folder_;
    }
}

// --- Keys of config.yaml ----------------------------------------------------
//
// To add a key (e.g. `language`): add a member with its default value to
// Configuration, a reader below and a line in kKeys, then document the key in
// config.example.yaml and in the README. A yaml-cpp exception thrown by a
// reader (e.g. `value.as<int>()`) becomes a ConfigurationException.

using KeyReader = void (*)(const YAML::Node &value,
                           const std::filesystem::path &config_file,
                           Configuration &configuration);

struct Key {
    std::string_view name_;
    KeyReader read_;
};

void ReadModuleFolder(const YAML::Node &value,
                      const std::filesystem::path &config_file,
                      Configuration &configuration) {
    SetModuleFolder(configuration,
                    ReadString(value, kModuleFolderKey, config_file),
                    config_file);
}

constexpr std::array kKeys{
    Key{kModuleFolderKey, &ReadModuleFolder},
};

Configuration FromYaml(const std::string &contents,
                       const std::filesystem::path &config_file) {
    Configuration configuration;
    try {
        const YAML::Node kRoot = YAML::Load(contents);
        if (kRoot.IsNull()) {
            return configuration;  // Empty file, or comments only.
        }
        if (!kRoot.IsMap()) {
            Fail(config_file,
                 "expected 'key: value' entries" + Where(kRoot.Mark()));
        }
        for (const auto &entry : kRoot) {
            const std::string kName =
                entry.first.IsScalar() ? entry.first.Scalar() : "";
            const auto kKey =
                std::ranges::find(kKeys, std::string_view{kName}, &Key::name_);
            if (kKey == kKeys.end()) {
                LOG_WARNING(
                    "Unknown key '{}' in configuration file '{}'{}, ignored",
                    kName, config_file.string(), Where(entry.first.Mark()));
                continue;
            }
            kKey->read_(entry.second, config_file, configuration);
        }
    } catch (const YAML::Exception &error) {
        Fail(config_file, error.msg + Where(error.mark));
    }
    return configuration;
}

// --- Deprecated config.json -------------------------------------------------
//
// Same rules as before the switch to YAML: an object whose only key is
// `module_folder`, a non-empty string. Remove with the JSON fallback.

Configuration FromLegacyJson(const std::string &contents,
                             const std::filesystem::path &config_file) {
    LOG_WARNING(
        "Configuration file '{}' uses the deprecated JSON format: replace it "
        "with '{}' (see config.example.yaml). JSON support will be removed.",
        config_file.string(), Configuration::kDefaultFile);
    nlohmann::json root;
    try {
        root = nlohmann::json::parse(contents);
    } catch (const nlohmann::json::exception &error) {
        Fail(config_file, error.what());
    }
    if (!root.is_object()) {
        Fail(config_file, "expected a JSON object");
    }
    Configuration configuration;
    for (const auto &[name, value] : root.items()) {
        if (name != kModuleFolderKey) {
            Fail(config_file, "unknown key '" + name + "'");
        }
        if (!value.is_string() ||
            value.get_ref<const std::string &>().empty()) {
            Fail(config_file, "'module_folder' must be a non-empty string");
        }
        SetModuleFolder(configuration, value.get<std::string>(), config_file);
    }
    return configuration;
}

// --- File access ------------------------------------------------------------

bool Exists(const std::filesystem::path &file) {
    std::error_code error;
    const bool kExists = std::filesystem::exists(file, error);
    if (error) {
        throw ConfigurationException("Unable to inspect configuration file '" +
                                     file.string() + "': " + error.message());
    }
    return kExists;
}

std::string ReadFile(const std::filesystem::path &file) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(file, error) || error) {
        throw ConfigurationException(
            "Configuration path is not a regular file: " + file.string());
    }
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        throw ConfigurationException("Unable to read configuration file '" +
                                     file.string() + "'");
    }
    return {std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
}

}  // namespace

std::optional<std::filesystem::path> Configuration::FileFromArguments(
    std::span<const std::string_view> arguments) {
    if (arguments.empty()) {
        return std::filesystem::path{kDefaultFile};
    }
    if (arguments.size() == 2 && arguments[0] == "--config") {
        return std::filesystem::path{arguments[1]};
    }
    return std::nullopt;
}

Configuration Configuration::Load(const std::filesystem::path &config_file) {
    std::filesystem::path file = config_file;
    if (!Exists(file) && file.extension() == ".yaml") {
        std::filesystem::path legacy = file;
        legacy.replace_extension(".json");
        if (Exists(legacy)) {
            file = std::move(legacy);
        }
    }
    if (!Exists(file)) {
        return Configuration{};
    }
    const std::string kContents = ReadFile(file);
    if (file.extension() == ".json") {
        return FromLegacyJson(kContents, file);
    }
    return FromYaml(kContents, file);
}

}  // namespace editor
