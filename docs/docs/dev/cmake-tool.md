# User Guide - CMake Compilation Tool

## Overview

The CMake compilation tool integrated into the editor allows you to quickly compile individual C++ files with customizable options, without having to manually manage the CMake configuration.

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [User Interface](#user-interface)
- [Compilation Options](#compilation-options)
- [Usage Examples](#usage-examples)
- [Troubleshooting](#troubleshooting)

## Quick Start

### Simple Compilation

1. Open a C++ file in the editor
2. Go to the **Tools** menu → **Compile with CMake**
3. Click on **Compile**
4. The executable will be generated in the build directory

### Compile and Run

1. Open your C++ source file
2. Go to the **Tools** menu → **Compile and Run**
3. The file will be compiled and then run automatically
4. The output will be displayed in the integrated terminal

## User Interface

### Compilation Configuration Panel

When you launch the CMake compilation tool, a configuration window opens with the following sections:

#### 1. **Source File**
- **Path**: Full path of the file to compile (automatically populated with the active file)
- **Browse Button**: Allows selecting another source file

#### 2. **General Configuration**

##### C++ Standard
Select the C++ standard to use:
- C++11 (ISO/IEC 14882:2011)
- C++14 (ISO/IEC 14882:2014)
- C++17 (ISO/IEC 14882:2017)
- C++20 (ISO/IEC 14882:2020)
- **C++23** (ISO/IEC 14882:2023) - *Default*

##### Build Type
Choose the compilation type:
- **Release**: Optimized for performance (default)
- **Debug**: With debugging symbols, no optimization
- **RelWithDebInfo**: Optimized with debugging information
- **MinSizeRel**: Optimized for binary size

##### Output Name
- **Text field**: Custom name for the executable
- If left empty, uses the source file name

#### 3. **Advanced Options**

##### Compilation Flags
List of flags passed to the compiler:
- **Add a flag**: `+` button to add a new flag
- **Examples**: `-Wall`, `-Wextra`, `-pedantic`, `-O3`, etc.
- **Remove**: Click on `✖` next to a flag to remove it

##### Linker Flags
List of options for the linker:
- **Add a flag**: `+` button to add
- **Examples**: `-lpthread`, `-lm`, `-static`, etc.

##### Include Directories
Paths for additional header files:
- **Add a path**: `+` button to add
- **Browse**: Select a directory via the file browser
- **Examples**: `/usr/local/include`, `./include`, `../external/headers`

##### Libraries
Names of libraries to link:
- **Add a library**: `+` button
- **Examples**: `pthread`, `m` (math), `boost_filesystem`
- **Note**: Do not include the `lib` prefix or the extension

##### Preprocessor Definitions
Macros and definitions for the preprocessor:
- **Add a definition**: `+` button
- **Format**: `NAME=value` or simply `NAME`
- **Examples**:
  - `DEBUG_MODE=1`
  - `VERSION=2.5`
  - `ENABLE_LOGGING`

#### 4. **Execution Options**

##### Build Directory
- **Path**: Location where CMake generates build files
- **Default**: `./cmake_build_<file_name>`
- **Browse button**: Choose a custom directory
- **Clean build**: Checkbox to delete the directory before compilation

##### Verbose Mode
- **Checkbox**: Enables detailed display of the compilation
- **Utility**: See all executed CMake and compiler commands

##### Run after compilation
- **Checkbox**: Automatically launches the executable after a successful compilation
- **Terminal**: Execution is displayed in the integrated terminal

#### 5. **Actions**

##### Compile Button
- Launches compilation with the configured options
- **Icon**: 🔨 or ⚙️
- **Shortcut**: `Ctrl+B` (configurable)

##### Compile and Run Button
- Equivalent to checking "Run after compilation" then compiling
- **Icon**: ▶️
- **Shortcut**: `Ctrl+Shift+B` (configurable)

##### Cancel Button
- Closes the panel without compiling
- **Shortcut**: `Esc`

##### Reset Button
- Restores default settings
- Keeps only the source file path

## Compilation Options

### Supported C++ Standards

| Standard | Description | Key features |
|----------|-------------|---------------------|
| C++11 | First modern standard | Lambdas, auto, move semantics |
| C++14 | Minor improvements | Generic lambdas, return type deduction |
| C++17 | Major evolution | std::optional, std::filesystem, structured bindings |
| C++20 | Modern standard | Concepts, ranges, coroutines, modules |
| C++23 | Latest standard | std::expected, multidimensional subscript, ranges improvements |

### Build Types Explained

#### Release (Production)
```
Optimizations: -O3
Debug symbols: No
Usage: Final release, maximum performance
Binary size: Medium
```

#### Debug (Development)
```
Optimizations: -O0
Debug symbols: Complete (-g)
Usage: Development, debugging with GDB/LLDB
Binary size: Large
```

#### RelWithDebInfo (Profiling)
```
Optimizations: -O2
Debug symbols: Yes (-g)
Usage: Profiling, performance analysis
Binary size: Large
```

#### MinSizeRel (Embedded)
```
Optimizations: -Os (size)
Debug symbols: No
Usage: Embedded systems, memory constraints
Binary size: Minimal
```

### Common Compilation Flags

#### Warnings
```
-Wall          # Enables basic warnings
-Wextra        # Additional warnings
-Werror        # Treats warnings as errors
-pedantic      # Strict adherence to the standard
-Wshadow       # Warns about shadowed variables
-Wconversion   # Warns about implicit conversions
```

#### Optimizations
```
-O0            # No optimization (debug)
-O1            # Basic optimizations
-O2            # Standard optimizations (recommended)
-O3            # Aggressive optimizations
-Os            # Optimization for size
-Ofast         # -O3 + non-IEEE compliant optimizations
-march=native  # Optimizations for current CPU
```

#### Debugging and Analysis
```
-g             # Debugging information
-g3            # Maximum debugging information
-fsanitize=address        # AddressSanitizer (memory leak detection)
-fsanitize=thread         # ThreadSanitizer (race conditions)
-fsanitize=undefined      # UndefinedBehaviorSanitizer
-fno-omit-frame-pointer   # Keeps frame pointers (profiling)
```

### Common Linker Flags

```
-lpthread      # POSIX threads library
-lm            # Math library
-ldl           # Dynamic loading
-static        # Static linking
-Wl,-rpath,.   # Runtime search path
```

## Usage Examples

### Example 1: Simple Program

**Context**: Compile a basic `hello.cpp` file

**Configuration**:
- C++ Standard: C++17
- Build Type: Release
- Advanced Options: None

**Result**: Executable `hello` in `./cmake_build_hello/bin/`

---

### Example 2: Program with Strict Warnings

**Context**: Production code requiring maximum quality

**Configuration**:
- C++ Standard: C++20
- Build Type: Release
- Compilation Flags:
  - `-Wall`
  - `-Wextra`
  - `-Werror`
  - `-pedantic`

**Result**: Compilation fails if warnings are present

---

### Example 3: Debugging with Sanitizer

**Context**: Memory leak search

**Configuration**:
- C++ Standard: C++17
- Build Type: Debug
- Compilation Flags:
  - `-fsanitize=address`
  - `-fno-omit-frame-pointer`
  - `-g`
- Linker Flags:
  - `-fsanitize=address`
- Run after compilation: ✓

**Result**: Program runs with AddressSanitizer active

---

### Example 4: Using External Libraries

**Context**: Program using Boost.Filesystem

**Configuration**:
- C++ Standard: C++17
- Build Type: Release
- Include Directories:
  - `/usr/local/include`
- Libraries:
  - `boost_filesystem`
  - `boost_system`
- Linker Flags:
  - `-lpthread`

---

### Example 5: Conditional Compilation

**Context**: Code with DEBUG/RELEASE sections

**Debug Configuration**:
- C++ Standard: C++20
- Build Type: Debug
- Definitions:
  - `DEBUG_MODE=1`
  - `LOG_LEVEL=VERBOSE`
  - `ENABLE_ASSERTIONS`

**Release Configuration**:
- C++ Standard: C++20
- Build Type: Release
- Definitions:
  - `NDEBUG`
  - `LOG_LEVEL=ERROR`

**Example code**:
```cpp
#ifdef DEBUG_MODE
    std::cout << "Debug: Variable value = " << var << std::endl;
#endif
```

---

### Example 6: Multi-file Project with Headers

**Structure**:
```
project/
├── src/
│   └── main.cpp
├── include/
│   ├── utils.hpp
│   └── config.hpp
└── lib/
    └── helper.cpp
```

**Configuration**:
- C++ Standard: C++23
- Build Type: Release
- Include Directories:
  - `./include`
  - `./lib`

**Note**: For multiple source files, create a static library or use a full CMakeLists.txt

---

### Example 7: Maximum Optimization

**Context**: Performance-critical code (scientific computing)

**Configuration**:
- C++ Standard: C++20
- Build Type: Release
- Compilation Flags:
  - `-O3`
  - `-march=native`
  - `-flto` (Link Time Optimization)
  - `-funroll-loops`
- Linker Flags:
  - `-flto`

---

### Example 8: Minimal Build for Embedded

**Context**: Embedded system with memory constraints

**Configuration**:
- C++ Standard: C++17
- Build Type: MinSizeRel
- Compilation Flags:
  - `-fno-exceptions`
  - `-fno-rtti`
  - `-ffunction-sections`
  - `-fdata-sections`
- Linker Flags:
  - `-Wl,--gc-sections`
  - `-static`

## Troubleshooting

### Common Issues

#### 1. "Header file not found"

**Symptom**:
```
fatal error: myheader.hpp: No such file or directory
```

**Solution**:
- Add the path in **Include Directories**
- Verify that the path is correct (absolute or relative to the source file)

#### 2. "Undefined reference" during linking

**Symptom**:
```
undefined reference to `pthread_create'
```

**Solution**:
- Add the missing library in **Libraries** (e.g., `pthread`)
- Or add the corresponding linker flag (e.g., `-lpthread`)

#### 3. "C++ feature not supported"

**Symptom**:
```
error: 'std::filesystem' is not a namespace-name
```

**Solution**:
- Increase the **C++ Standard** (e.g., C++17 minimum for std::filesystem)
- Check your compiler's compatibility

#### 4. Compilation successful but executable not found

**Symptom**:
```
Compilation completed but executable not found
```

**Solution**:
- Check the **Build Directory**
- Try checking **Clean build** before compilation
- Check file system permissions

#### 5. Sanitizer errors during execution

**Symptom**:
```
AddressSanitizer: heap-use-after-free
```

**Solution**:
- This is not a tool error but an error detected in your code
- Use the provided information to locate the bug
- Compile with `-g` to get precise line numbers

#### 6. Slow compilation

**Solution**:
- Do not check **Clean build** for incremental compilations
- Reduce the optimization level during development (use Debug)
- Disable **Verbose Mode** unless there is an issue

#### 7. Message "CMake configuration failed"

**Symptom**:
```
CMake Error: Could not create named generator
```

**Solution**:
- Verify that CMake is installed (`cmake --version`)
- Verify that the C++ compiler is accessible
- Try to clean the build directory

## Keyboard Shortcuts

| Action | Shortcut | Description |
|--------|-----------|-------------|
| Compile the active file | `Ctrl+B` | Opens the compilation panel |
| Compile and run | `Ctrl+Shift+B` | Compiles then executes |
| Stop compilation | `Ctrl+C` | Stops the running process |
| Close the panel | `Esc` | Closes without compiling |
| Navigate between fields | `Tab` | Moves to the next field |
| Enable/disable option | `Space` | For checkboxes |

## Tips and Best Practices

### 1. Compilation Profiles

Create reusable profiles for your common configurations:
- **Dev**: C++20, Debug, -Wall -Wextra
- **Prod**: C++20, Release, -O3 -Wall -Werror
- **Profile**: C++20, RelWithDebInfo, -fno-omit-frame-pointer

### 2. Recommended Flags by Use Case

**Daily development**:
```
-Wall -Wextra -g
```

**Production code**:
```
-Wall -Wextra -Werror -O3 -DNDEBUG
```

**Bug hunting**:
```
-Wall -Wextra -g -fsanitize=address -fsanitize=undefined
```

**Critical performance**:
```
-O3 -march=native -flto -DNDEBUG
```

### 3. Build Organization

- Use separate build directories for Debug/Release
- Example: `./cmake_build_debug` and `./cmake_build_release`
- Do not version build directories in Git

### 4. Incremental Compilation

- Only clean the build if necessary
- CMake will only rebuild modified files
- Considerable time savings on large projects

### 5. Verbose Mode

- Enable it to understand what happens in case of an error
- Disable it for fast compilations without issues
- Useful for debugging linking issues

## Current Limitations

### Single Source File
The tool is designed to compile **a single source file** at a time. For complex multi-file projects:
- Use a complete `CMakeLists.txt` in your project
- Or create a static library and then link it

### No Configuration Cache
Settings are not saved between sessions. To reuse a configuration:
- Create custom profiles (planned future feature)
- Or use the CLI tool with a script

### Cross-compilation
Cross-compilation is not supported via the graphical interface. Use CMake directly for these advanced cases.

## Support and Resources

### Technical Documentation
- [CMake Official Documentation](https://cmake.org/documentation/)
- [GCC Compiler Options](https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html)
- [Clang Compiler Options](https://clang.llvm.org/docs/ClangCommandLineReference.html)

### C++ Standards
- [C++17 Reference](https://en.cppreference.com/w/cpp/17)
- [C++20 Reference](https://en.cppreference.com/w/cpp/20)
- [C++23 Reference](https://en.cppreference.com/w/cpp/23)

### Debugging Tools
- [AddressSanitizer Documentation](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [GDB Tutorial](https://www.gnu.org/software/gdb/documentation/)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)

---

**Version**: 1.0
**Last update**: March 2026
**Author**: Nebula Editor Team
