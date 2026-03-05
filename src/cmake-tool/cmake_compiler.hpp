#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace nebula::cmake {

/**
 * @brief Configuration options for CMake compilation
 */
struct CompilerConfig {
    std::string cxx_standard = "23";
    std::string build_type = "Release";
    std::vector<std::string> compile_flags;
    std::vector<std::string> link_flags;
    std::vector<std::string> include_directories;
    std::vector<std::string> libraries;
    std::vector<std::string> definitions;
    std::string output_name;
    bool verbose = false;
};

/**
 * @brief Result of a compilation
 */
struct CompilationResult {
    bool success;
    std::string output;
    std::string error_output;
    std::filesystem::path executable_path;
    int exit_code;
};

/**
 * @brief CMake-based compiler for single C++ files
 *
 * This class provides functionality similar to Visual Studio's CMake tools
 * for compiling single C++ files with custom arguments.
 */
class CMakeCompiler {
   public:
    CMakeCompiler() = default;
    ~CMakeCompiler() = default;

    /**
     * @brief Compile a single C++ source file
     * @param source_file Path to the C++ source file
     * @param config Compilation configuration
     * @return CompilationResult with success status and details
     */
    CompilationResult compile_file(const std::filesystem::path &source_file,
                                   const CompilerConfig &config);

    /**
     * @brief Set the build directory for temporary CMake files
     * @param dir Build directory path
     */
    void set_build_directory(const std::filesystem::path &dir);

    /**
     * @brief Get the current build directory
     * @return Path to the build directory
     */
    std::filesystem::path get_build_directory() const;

    /**
     * @brief Clean the build directory
     */
    void clean_build_directory();

   private:
    std::filesystem::path build_dir_ =
        std::filesystem::temp_directory_path() / "nebula_cmake_build";

    /**
     * @brief Generate CMakeLists.txt for the source file
     */
    std::string generate_cmake_file(const std::filesystem::path &source_file,
                                    const CompilerConfig &config);

    /**
     * @brief Execute a system command and capture output
     */
    CompilationResult execute_command(const std::string &command,
                                      const std::filesystem::path &working_dir);

    /**
     * @brief Create the build directory if it doesn't exist
     */
    void ensure_build_directory();
};

}  // namespace nebula::cmake
