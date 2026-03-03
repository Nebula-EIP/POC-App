cmake_minimum_required(VERSION 3.20)

project(Nebula
    VERSION 0.1.0
    LANGUAGES CXX C
)

# Global settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CPM source cache for CI/CD optimization
set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cache/CPM" CACHE PATH "CPM source cache")

file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.1/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=65cf5ff1bfb6b3f77e36f9d25cf0b98d2ddc17f37eff63d8c19c06c84c04c98b
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

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
