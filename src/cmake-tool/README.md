# CMake Tool Module

This module provides CMake-based compilation functionality for single C++ files, similar to Visual Studio's CMake tools.

## Features

- Compile single C++ source files with custom arguments
- Configurable C++ standard (default: C++23)
- Multiple build types (Debug, Release, RelWithDebInfo, MinSizeRel)
- Custom compile and link flags
- Include directories and library linking
- Preprocessor definitions
- Verbose output mode
- Clean build option
- Run executable after compilation

## Usage

### As a Library

```cpp
#include "cmake_compiler.hpp"

nebula::cmake::CMakeCompiler compiler;
nebula::cmake::CompilerConfig config;

config.cxx_standard = "23";
config.build_type = "Release";
config.compile_flags = {"-Wall", "-Wextra"};
config.verbose = true;

auto result = compiler.compile_file("main.cpp", config);

if (result.success) {
    std::cout << "Executable: " << result.executable_path << "\n";
} else {
    std::cerr << "Error: " << result.error_output << "\n";
}
```

### As a CLI Tool

```bash
# Simple compilation
./cmake-tool main.cpp

# With options
./cmake-tool main.cpp --std=20 --build-type=Debug --verbose

# With custom flags and libraries
./cmake-tool main.cpp --flag=-Wall --flag=-Wextra --library=pthread

# Compile and run
./cmake-tool main.cpp --run

# Full example
./cmake-tool main.cpp \
    --std=23 \
    --build-type=Release \
    --output=myapp \
    --flag=-Wall \
    --flag=-Wextra \
    --include=/usr/local/include \
    --library=pthread \
    --define=DEBUG_MODE \
    --verbose \
    --run
```

## API Reference

### CompilerConfig

Configuration structure for compilation:

- `cxx_standard`: C++ standard version (default: "23")
- `build_type`: Build type (default: "Release")
- `compile_flags`: Vector of compilation flags
- `link_flags`: Vector of linker flags
- `include_directories`: Vector of include directories
- `libraries`: Vector of libraries to link
- `definitions`: Vector of preprocessor definitions
- `output_name`: Custom executable name
- `verbose`: Enable verbose output

### CMakeCompiler

Main compiler class:

- `compile_file(source_file, config)`: Compile a source file
- `set_build_directory(dir)`: Set custom build directory
- `get_build_directory()`: Get current build directory
- `clean_build_directory()`: Clean the build directory

### CompilationResult

Result of compilation:

- `success`: Whether compilation succeeded
- `output`: Standard output from compilation
- `error_output`: Error messages if any
- `executable_path`: Path to compiled executable
- `exit_code`: Exit code of compilation process
