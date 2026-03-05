#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "cmake_compiler.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

using ::testing::HasSubstr;

class CMakeCompilerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test directory
        test_dir_ = fs::temp_directory_path() / "cmake_compiler_test";
        fs::create_directories(test_dir_);

        // Create a simple test file
        test_file_ = test_dir_ / "test.cpp";
        std::ofstream out(test_file_);
        out << "#include <iostream>\n";
        out << "int main() {\n";
        out << "    std::cout << \"Test\" << std::endl;\n";
        out << "    return 0;\n";
        out << "}\n";
        out.close();
    }

    void TearDown() override {
        // Clean up test directory
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    fs::path test_dir_;
    fs::path test_file_;
};

// Test default configuration
TEST_F(CMakeCompilerTest, DefaultConfiguration) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;

    EXPECT_EQ(config.cxx_standard, "23");
    EXPECT_EQ(config.build_type, "Release");
    EXPECT_FALSE(config.verbose);
    EXPECT_TRUE(config.compile_flags.empty());
    EXPECT_TRUE(config.libraries.empty());
}

// Test setting build directory
TEST_F(CMakeCompilerTest, SetBuildDirectory) {
    nebula::cmake::CMakeCompiler compiler;
    fs::path custom_dir = test_dir_ / "custom_build";

    compiler.set_build_directory(custom_dir);
    EXPECT_EQ(compiler.get_build_directory(), custom_dir);
}

// Test compilation with valid source file
TEST_F(CMakeCompilerTest, CompileValidSourceFile) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;

    auto build_dir = test_dir_ / "build";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(test_file_, config);

    EXPECT_TRUE(result.success) << "Error: " << result.error_output;
    EXPECT_TRUE(fs::exists(result.executable_path));
    EXPECT_EQ(result.exit_code, 0);
}

// Test compilation with non-existent file
TEST_F(CMakeCompilerTest, CompileNonExistentFile) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;

    fs::path non_existent = test_dir_ / "does_not_exist.cpp";

    auto result = compiler.compile_file(non_existent, config);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_output.empty());
    EXPECT_THAT(result.error_output, testing::HasSubstr("does not exist"));
}

// Test compilation with custom C++ standard
TEST_F(CMakeCompilerTest, CustomCxxStandard) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;
    config.cxx_standard = "17";

    auto build_dir = test_dir_ / "build_std17";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(test_file_, config);

    EXPECT_TRUE(result.success);
}

// Test compilation with Debug build type
TEST_F(CMakeCompilerTest, DebugBuildType) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;
    config.build_type = "Debug";

    auto build_dir = test_dir_ / "build_debug";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(test_file_, config);

    EXPECT_TRUE(result.success);
}

// Test compilation with compile flags
TEST_F(CMakeCompilerTest, WithCompileFlags) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;
    config.compile_flags = {"-Wall", "-Wextra"};

    auto build_dir = test_dir_ / "build_flags";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(test_file_, config);

    EXPECT_TRUE(result.success);
}

// Test compilation with custom output name
TEST_F(CMakeCompilerTest, CustomOutputName) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;
    config.output_name = "custom_test";

    auto build_dir = test_dir_ / "build_custom";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(test_file_, config);

    EXPECT_TRUE(result.success);
    EXPECT_THAT(result.executable_path.string(), testing::HasSubstr("custom_test"));
}

// Test compilation with definitions
TEST_F(CMakeCompilerTest, WithDefinitions) {
    // Create a source file that uses a definition
    fs::path def_test_file = test_dir_ / "def_test.cpp";
    std::ofstream out(def_test_file);
    out << "#include <iostream>\n";
    out << "int main() {\n";
    out << "#ifdef MY_DEFINE\n";
    out << "    std::cout << \"Defined\" << std::endl;\n";
    out << "#else\n";
    out << "    std::cout << \"Not defined\" << std::endl;\n";
    out << "#endif\n";
    out << "    return 0;\n";
    out << "}\n";
    out.close();

    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;
    config.definitions = {"MY_DEFINE"};

    auto build_dir = test_dir_ / "build_def";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(def_test_file, config);

    EXPECT_TRUE(result.success);
}

// Test compilation with include directories
TEST_F(CMakeCompilerTest, WithIncludeDirectories) {
    // Create a header file
    fs::path include_dir = test_dir_ / "include";
    fs::create_directories(include_dir);

    fs::path header_file = include_dir / "myheader.hpp";
    std::ofstream header(header_file);
    header << "#pragma once\n";
    header << "void test_function();\n";
    header.close();

    // Create source file that includes the header
    fs::path source_file = test_dir_ / "with_include.cpp";
    std::ofstream src(source_file);
    src << "#include <myheader.hpp>\n";
    src << "#include <iostream>\n";
    src << "void test_function() { std::cout << \"Test\" << std::endl; }\n";
    src << "int main() { test_function(); return 0; }\n";
    src.close();

    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;
    config.include_directories = {include_dir.string()};

    auto build_dir = test_dir_ / "build_include";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(source_file, config);

    EXPECT_TRUE(result.success);
}

// Test clean build directory
TEST_F(CMakeCompilerTest, CleanBuildDirectory) {
    nebula::cmake::CMakeCompiler compiler;
    auto build_dir = test_dir_ / "build_clean";
    compiler.set_build_directory(build_dir);

    // Create some files in the build directory
    fs::create_directories(build_dir);
    std::ofstream dummy(build_dir / "dummy.txt");
    dummy << "test";
    dummy.close();

    EXPECT_TRUE(fs::exists(build_dir / "dummy.txt"));

    compiler.clean_build_directory();

    EXPECT_FALSE(fs::exists(build_dir / "dummy.txt"));
}

// Test compilation with invalid C++ code
TEST_F(CMakeCompilerTest, InvalidCppCode) {
    fs::path invalid_file = test_dir_ / "invalid.cpp";
    std::ofstream out(invalid_file);
    out << "#include <iostream>\n";
    out << "int main() {\n";
    out << "    this is not valid C++ code\n";
    out << "    return 0;\n";
    out << "}\n";
    out.close();

    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;

    auto build_dir = test_dir_ / "build_invalid";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(invalid_file, config);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_output.empty());
}

// Test multiple compilations in sequence
TEST_F(CMakeCompilerTest, MultipleCompilations) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;

    auto build_dir1 = test_dir_ / "build1";
    compiler.set_build_directory(build_dir1);
    auto result1 = compiler.compile_file(test_file_, config);
    EXPECT_TRUE(result1.success);

    auto build_dir2 = test_dir_ / "build2";
    compiler.set_build_directory(build_dir2);
    auto result2 = compiler.compile_file(test_file_, config);
    EXPECT_TRUE(result2.success);

    // Both executables should exist
    EXPECT_TRUE(fs::exists(result1.executable_path));
    EXPECT_TRUE(fs::exists(result2.executable_path));
}

// Test compilation with all options combined
TEST_F(CMakeCompilerTest, AllOptionsCombined) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;

    config.cxx_standard = "20";
    config.build_type = "Debug";
    config.compile_flags = {"-Wall", "-Wextra", "-Wpedantic"};
    config.definitions = {"DEBUG_MODE", "TEST_BUILD"};
    config.output_name = "full_test";
    config.verbose = false;

    auto build_dir = test_dir_ / "build_all";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(test_file_, config);

    EXPECT_TRUE(result.success);
    EXPECT_THAT(result.executable_path.string(), testing::HasSubstr("full_test"));
}

// Test that generated CMakeLists.txt contains correct content
TEST_F(CMakeCompilerTest, GeneratedCMakeListsContent) {
    nebula::cmake::CMakeCompiler compiler;
    nebula::cmake::CompilerConfig config;
    config.cxx_standard = "17";
    config.compile_flags = {"-Wall"};

    auto build_dir = test_dir_ / "build_cmake";
    compiler.set_build_directory(build_dir);

    auto result = compiler.compile_file(test_file_, config);
    EXPECT_TRUE(result.success);

    // Check if CMakeLists.txt was created
    fs::path cmake_file = build_dir / "CMakeLists.txt";
    EXPECT_TRUE(fs::exists(cmake_file));

    // Read and verify content
    std::ifstream cmake_in(cmake_file);
    std::string content((std::istreambuf_iterator<char>(cmake_in)),
                        std::istreambuf_iterator<char>());

    EXPECT_THAT(content, testing::HasSubstr("CMAKE_CXX_STANDARD 17"));
    EXPECT_THAT(content, testing::HasSubstr("-Wall"));

    std::string expected_path = test_file_.string();
    std::replace(expected_path.begin(), expected_path.end(), '\\', '/');
    EXPECT_THAT(content, testing::HasSubstr(expected_path));
}
