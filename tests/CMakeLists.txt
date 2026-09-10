# Tests CMakeLists.txt

# Collect test files from new_core only
file(GLOB_RECURSE TEST_SOURCES "new_core/*.cpp")

# Create test executable
if(TEST_SOURCES)
    add_executable(unit_tests ${TEST_SOURCES})

    # DISABLED: Code-generation depends on old core library
    # Removed target_sources for code-generation files

    # Allow includes for new_core tests
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/new_core
    )

    # Link with project libraries when available
    target_link_libraries(unit_tests PRIVATE
        gtest
        gtest_main
        gmock
        neb_graph
        new_core_capabilities
    )

    # Auto-discover all Google Test tests
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
