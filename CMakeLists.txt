cmake_minimum_required(VERSION 3.20)

project(Nebula
    VERSION 0.1.0
    LANGUAGES CXX C
)

# Global settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CPM source cache for CI/CD optimization
set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cache/CPM" CACHE PATH "CPM source cache")

# Include CPM from local cmake directory
include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

# Add nlohmann/json for JSON serialization
CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.11.3
    GITHUB_REPOSITORY nlohmann/json
    OPTIONS "JSON_BuildTests OFF" "JSON_Install OFF"
)

# Testing support
option(BUILD_TESTING "Build the testing tree." ON)

if(BUILD_TESTING)
    enable_testing()
    CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.14.0
        OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt ON"
    )
endif()

# Add subdirectories
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Only for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: enabling automatic analysis")
        set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}")
    else()
        message(STATUS "clang-tidy not found: skipping automatic analysis")
    endif()
endif()
