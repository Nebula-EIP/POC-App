# Tests CMakeLists.txt

# Collect test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Create test executable
if(TEST_SOURCES)
    add_executable(nebula_tests ${TEST_SOURCES})

    target_compile_features(nebula_tests PRIVATE cxx_std_20)

    # Code-generation lives under src/editor and isn't a standalone library yet.
    # Compile the code-generation sources into the test executable.
    target_sources(nebula_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file-formatting.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/file-writing/file-writer.cpp
    )

    # Allow includes like "code-generation/..." and "errors/...".
    target_include_directories(nebula_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor
    )

    # code-generator-file-formatting depends on utils/string-splitter.hpp
    target_link_libraries(nebula_tests PRIVATE utils)

    # Link with project libraries when available
    target_link_libraries(nebula_tests PRIVATE
        gtest
        gtest_main
        gmock
        core
        cmake_tool
        #utils
    )

    # Register tests with CTest
    add_test(NAME NebulaCoreTests COMMAND nebula_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
