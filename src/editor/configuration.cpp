/**
 * @file Configuration.cpp
 * @brief Implementation of the Configuration class, which handles the loading
 *        and parsing of configuration files for the editor.
 *
 * @author Created by Nolan Papa
 * @date Created on 12-10-2026
 *
 * @author Last modified by Nolan Papa
 * @date Last modified on 12-10-2026
 */

#include "configuration.hpp"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string ReadJsonString(std::istringstream &input,
                           const std::filesystem::path &config_file) {
    std::string value;
    if (!(input >> std::quoted(value))) {
        throw std::runtime_error(
            "Configuration value 'module_folder' must be "
            "a string in '" +
            config_file.string() + "'");
    }
    return value;
}

}  // namespace

namespace editor {

Configuration Configuration::Load(const std::filesystem::path &config_file) {
    Configuration configuration{std::filesystem::path{kDefaultModuleFolder}};
    std::error_code error;
    if (!std::filesystem::exists(config_file, error)) {
        if (error) {
            throw std::runtime_error("Unable to inspect configuration file '" +
                                     config_file.string() +
                                     "': " + error.message());
        }
        return configuration;
    }
    if (!std::filesystem::is_regular_file(config_file, error) || error) {
        throw std::runtime_error("Configuration path is not a regular file: " +
                                 config_file.string());
    }

    std::ifstream input(config_file);
    const std::string kContents((std::istreambuf_iterator<char>(input)),
                                std::istreambuf_iterator<char>());
    std::istringstream json(kContents);
    char token = '\0';
    std::string key;
    if (!(json >> token) || token != '{' || !(json >> std::quoted(key)) ||
        key != "module_folder" || !(json >> token) || token != ':') {
        throw std::runtime_error("Invalid configuration file: " +
                                 config_file.string());
    }

    const std::string kModuleFolder = ReadJsonString(json, config_file);
    if (kModuleFolder.empty()) {
        throw std::runtime_error(
            "Configuration value 'module_folder' must not be empty");
    }
    if (!(json >> token) || token != '}' || (json >> token)) {
        throw std::runtime_error("Invalid configuration file: " +
                                 config_file.string());
    }
    configuration.module_folder_ = kModuleFolder;
    if (configuration.module_folder_.is_relative()) {
        configuration.module_folder_ =
            config_file.parent_path() / configuration.module_folder_;
    }
    return configuration;
}

}  // namespace editor
