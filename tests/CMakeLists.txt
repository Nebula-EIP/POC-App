# Tests CMakeLists.txt

# Collect test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Create test executable
if(TEST_SOURCES)
    add_executable(unit_tests ${TEST_SOURCES})

    # Code-generation lives under src/editor and isn't a standalone library yet.
    # Compile the code-generation sources into the test executable.
    target_sources(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file-formatting.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/codegen-context.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/file-writing/file-writer.cpp
    )

    # Allow includes like "code-generation/..." and "errors/...".
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor
    )

    # code-generator-file-formatting depends on utils/string-splitter.hpp
    target_link_libraries(unit_tests PRIVATE utils)

    # Link with project libraries when available
    target_link_libraries(unit_tests PRIVATE
        gtest
        gtest_main
        gmock
        core
        cmake_tool
        #utils
    )

    # Auto-discover all Google Test tests
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
