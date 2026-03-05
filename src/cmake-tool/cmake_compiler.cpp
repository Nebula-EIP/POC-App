#include "cmake_compiler.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/wait.h>
#endif

namespace nebula::cmake {

void CMakeCompiler::SetBuildDirectory(const std::filesystem::path &dir) {
    build_dir = dir;
}

std::filesystem::path CMakeCompiler::GetBuildDirectory() const {
    return build_dir;
}

void CMakeCompiler::EnsureBuildDirectory() {
    if (!std::filesystem::exists(build_dir)) {
        std::filesystem::create_directories(build_dir);
    }
}

void CMakeCompiler::CleanBuildDirectory() {
    if (std::filesystem::exists(build_dir)) {
        std::filesystem::remove_all(build_dir);
    }
}

std::string CMakeCompiler::GenerateCmakeFile(
    const std::filesystem::path &source_file, const CompilerConfig &config) {
    std::stringstream cmake_content;

    cmake_content << "cmake_minimum_required(VERSION 3.20)\n";
    cmake_content << "project(NebulaCompiledFile LANGUAGES CXX)\n\n";

    cmake_content << "set(CMAKE_CXX_STANDARD " << config.cxx_standard << ")\n";
    cmake_content << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    cmake_content << "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";

    cmake_content << "set(CMAKE_BUILD_TYPE " << config.build_type << ")\n\n";

    if (!config.compile_flags.empty()) {
        cmake_content << "add_compile_options(";
        for (size_t i = 0; i < config.compile_flags.size(); ++i) {
            if (i > 0) cmake_content << " ";
            cmake_content << config.compile_flags[i];
        }
        cmake_content << ")\n\n";
    }

    if (!config.include_directories.empty()) {
        cmake_content << "include_directories(";
        for (size_t i = 0; i < config.include_directories.size(); ++i) {
            if (i > 0) cmake_content << " ";
            std::string include_path = config.include_directories[i];
            std::replace(include_path.begin(), include_path.end(), '\\', '/');
            cmake_content << include_path;
        }
        cmake_content << ")\n\n";
    }

    if (!config.definitions.empty()) {
        for (const auto &def : config.definitions) {
            cmake_content << "add_definitions(-D" << def << ")\n";
        }
        cmake_content << "\n";
    }

    std::string exec_name = config.output_name.empty()
                                ? source_file.stem().string()
                                : config.output_name;

    std::string source_path = std::filesystem::absolute(source_file).string();
    std::replace(source_path.begin(), source_path.end(), '\\', '/');

    cmake_content << "add_executable(" << exec_name << " " << source_path
                  << ")\n\n";

    if (!config.link_flags.empty()) {
        cmake_content << "target_link_options(" << exec_name << " PRIVATE ";
        for (size_t i = 0; i < config.link_flags.size(); ++i) {
            if (i > 0) cmake_content << " ";
            cmake_content << config.link_flags[i];
        }
        cmake_content << ")\n\n";
    }

    if (!config.libraries.empty()) {
        cmake_content << "target_link_libraries(" << exec_name << " ";
        for (size_t i = 0; i < config.libraries.size(); ++i) {
            if (i > 0) cmake_content << " ";
            cmake_content << config.libraries[i];
        }
        cmake_content << ")\n\n";
    }

    cmake_content << "set_target_properties(" << exec_name << " PROPERTIES\n";
    cmake_content
        << "    RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_BINARY_DIR}/bin\"\n";
    cmake_content << ")\n";

    return cmake_content.str();
}

CompilationResult CMakeCompiler::ExecuteCommand(
    const std::string &command, const std::filesystem::path &working_dir) {
    CompilationResult result;
    result.success = false;
    result.exit_code = -1;

    auto original_path = std::filesystem::current_path();
    try {
        std::filesystem::current_path(working_dir);
    } catch (const std::exception &e) {
        result.error_output =
            "Failed to change to working directory: " + std::string(e.what());
        return result;
    }

    // Execute command and capture output
    std::array<char, 128> buffer;
    std::string output;

#ifdef _WIN32
    FILE *pipe = POPEN(command.c_str(), "r");
#else
    FILE *pipe = POPEN((command + " 2>&1").c_str(), "r");
#endif

    if (!pipe) {
        result.error_output = "Failed to execute command: " + command;
        std::filesystem::current_path(original_path);
        return result;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

    int status = PCLOSE(pipe);

#ifdef _WIN32
    // On Windows, _pclose returns the exit code directly
    result.exit_code = status;
#else
    // On Unix/Linux, use WEXITSTATUS to extract exit code
    result.exit_code = WEXITSTATUS(status);
#endif

    result.success = (result.exit_code == 0);

    if (result.success) {
        result.output = output;
    } else {
        result.error_output = output;
    }

    std::filesystem::current_path(original_path);

    return result;
}

CompilationResult CMakeCompiler::CompileFile(
    const std::filesystem::path &source_file, const CompilerConfig &config) {
    CompilationResult final_result;
    final_result.success = false;
    final_result.exit_code = -1;

    if (!std::filesystem::exists(source_file)) {
        final_result.error_output =
            "Source file does not exist: " + source_file.string();
        return final_result;
    }

    EnsureBuildDirectory();

    // Generate CMakeLists.txt
    std::string cmake_content = GenerateCmakeFile(source_file, config);
    std::filesystem::path cmake_file = build_dir / "CMakeLists.txt";

    std::ofstream cmake_out(cmake_file);
    if (!cmake_out) {
        final_result.error_output = "Failed to create CMakeLists.txt";
        return final_result;
    }
    cmake_out << cmake_content;
    cmake_out.close();

    if (config.verbose) {
        final_result.output += "=== Generated CMakeLists.txt ===\n";
        final_result.output += cmake_content;
        final_result.output += "\n=================================\n\n";
    }

    std::string cmake_command = "cmake";
    if (config.verbose) {
        cmake_command += " --trace-expand";
    }
    cmake_command += " -S . -B .";

    if (config.verbose) {
        final_result.output += "=== Running CMake Configuration ===\n";
        final_result.output += "Command: " + cmake_command + "\n\n";
    }

    CompilationResult cmake_result = ExecuteCommand(cmake_command, build_dir);
    if (!cmake_result.success) {
        final_result.error_output =
            "CMake configuration failed:\n" + cmake_result.error_output;
        return final_result;
    }

    if (config.verbose) {
        final_result.output += cmake_result.output + "\n";
    }

    std::string build_command = "cmake --build . --config " + config.build_type;
    if (config.verbose) {
        build_command += " --verbose";
    }

    if (config.verbose) {
        final_result.output += "=== Running CMake Build ===\n";
        final_result.output += "Command: " + build_command + "\n\n";
    }

    CompilationResult build_result = ExecuteCommand(build_command, build_dir);
    if (!build_result.success) {
        final_result.error_output =
            "Build failed:\n" + build_result.error_output;
        return final_result;
    }

    final_result.output += build_result.output;
    std::string exec_name = config.output_name.empty()
                                ? source_file.stem().string()
                                : config.output_name;

#ifdef _WIN32
    if (exec_name.find(".exe") == std::string::npos) {
        exec_name += ".exe";
    }
#endif

    std::vector<std::filesystem::path> possible_paths = {
        build_dir / "bin" / exec_name,
        build_dir / "bin" / config.build_type / exec_name,
        build_dir / config.build_type / exec_name, build_dir / exec_name};

    bool found = false;
    for (const auto &path : possible_paths) {
        if (std::filesystem::exists(path)) {
            final_result.executable_path = path;
            found = true;
            break;
        }
    }

    if (found) {
        final_result.success = true;
        final_result.exit_code = 0;
        final_result.output += "\n=== Compilation Successful ===\n";
        final_result.output +=
            "Executable: " + final_result.executable_path.string() + "\n";
    } else {
        final_result.success = false;
        final_result.error_output =
            "Compilation completed but executable not found";
    }

    return final_result;
}

}  // namespace nebula::cmake
