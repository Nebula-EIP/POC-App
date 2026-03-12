# Tests CMakeLists.txt

# Collect test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Create test executable
if(TEST_SOURCES)
    add_executable(nebula_tests ${TEST_SOURCES})

    target_compile_features(nebula_tests PRIVATE cxx_std_20)

    # Allow includes like "file-writing/...".
    target_include_directories(nebula_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor
    )

    # Compile editor sources that aren't in a library yet.
    target_sources(nebula_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor/file-writing/file-writer.cpp
    )

    # Link with project libraries
    target_link_libraries(nebula_tests PRIVATE
        gtest
        gtest_main
        gmock
        core
        cmake_tool
        graph-exporter
    )

    # Register tests with CTest
    add_test(NAME NebulaCoreTests COMMAND nebula_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
