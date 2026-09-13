# Tests CMakeLists.txt

# Collect test files from core only
file(GLOB_RECURSE TEST_SOURCES "core/*.cpp")

# Create test executable
if(TEST_SOURCES)
    add_executable(unit_tests ${TEST_SOURCES})

    # Allow includes for core tests
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
    )

    # Link with project libraries when available
    target_link_libraries(unit_tests PRIVATE
        project_warnings
        gtest
        gtest_main
        gmock
        neb_graph
        core_capabilities
    )

    # Auto-discover all Google Test tests
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
