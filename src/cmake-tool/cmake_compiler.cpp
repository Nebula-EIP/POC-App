#include "cmake_compiler.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
    #define WEXITSTATUS(x) (x)
#else
    #include <sys/wait.h>
#endif

namespace nebula::cmake {

void CMakeCompiler::set_build_directory(const std::filesystem::path &dir) {
    build_dir_ = dir;
}

std::filesystem::path CMakeCompiler::get_build_directory() const {
    return build_dir_;
}

void CMakeCompiler::ensure_build_directory() {
    if (!std::filesystem::exists(build_dir_)) {
        std::filesystem::create_directories(build_dir_);
    }
}

void CMakeCompiler::clean_build_directory() {
    if (std::filesystem::exists(build_dir_)) {
        std::filesystem::remove_all(build_dir_);
    }
}

std::string CMakeCompiler::generate_cmake_file(
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
            cmake_content << config.include_directories[i];
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

    cmake_content << "add_executable(" << exec_name << " "
                  << std::filesystem::absolute(source_file).string() << ")\n\n";

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

CompilationResult CMakeCompiler::execute_command(
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

    FILE *pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
        result.error_output = "Failed to execute command: " + command;
        std::filesystem::current_path(original_path);
        return result;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

    int status = pclose(pipe);
    result.exit_code = WEXITSTATUS(status);
    result.success = (result.exit_code == 0);

    if (result.success) {
        result.output = output;
    } else {
        result.error_output = output;
    }

    std::filesystem::current_path(original_path);

    return result;
}

CompilationResult CMakeCompiler::compile_file(
    const std::filesystem::path &source_file, const CompilerConfig &config) {
    CompilationResult final_result;
    final_result.success = false;
    final_result.exit_code = -1;

    if (!std::filesystem::exists(source_file)) {
        final_result.error_output =
            "Source file does not exist: " + source_file.string();
        return final_result;
    }

    ensure_build_directory();

    // Generate CMakeLists.txt
    std::string cmake_content = generate_cmake_file(source_file, config);
    std::filesystem::path cmake_file = build_dir_ / "CMakeLists.txt";

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

    CompilationResult cmake_result = execute_command(cmake_command, build_dir_);
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

    CompilationResult build_result = execute_command(build_command, build_dir_);
    if (!build_result.success) {
        final_result.error_output =
            "Build failed:\n" + build_result.error_output;
        return final_result;
    }

    final_result.output += build_result.output;
    std::string exec_name = config.output_name.empty()
                                ? source_file.stem().string()
                                : config.output_name;

    final_result.executable_path = build_dir_ / "bin" / exec_name;

    if (!std::filesystem::exists(final_result.executable_path)) {
        final_result.executable_path = build_dir_ / exec_name;
    }

    if (std::filesystem::exists(final_result.executable_path)) {
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
