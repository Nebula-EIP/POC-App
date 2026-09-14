# Tests CMakeLists.txt

# Shared library fixtures loaded at runtime by the ModuleLoader tests.
add_subdirectory(core/modules/fixtures)

# Collect test files from core only
file(GLOB_RECURSE TEST_SOURCES "core/**.cpp")

# The module fixtures are shared libraries of their own, they must never be
# compiled into the test executable.
list(FILTER TEST_SOURCES EXCLUDE REGEX "/core/modules/fixtures/")

# Create test executable
if(TEST_SOURCES)
    add_executable(unit_tests ${TEST_SOURCES} modules/c/c_module_tests.cpp)

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
        core_modules
    )

    # The loader tests open real shared libraries, so they need their absolute
    # paths. Generator expressions keep this correct for multi-config builds.
    target_compile_definitions(unit_tests PRIVATE
        NEBULA_FIXTURE_VALID="$<TARGET_FILE:fixture_valid_module>"
        NEBULA_FIXTURE_SECOND="$<TARGET_FILE:fixture_second_module>"
        NEBULA_FIXTURE_RENDERER="$<TARGET_FILE:fixture_renderer_module>"
        NEBULA_FIXTURE_NO_FACTORY="$<TARGET_FILE:fixture_no_factory_module>"
        NEBULA_FIXTURE_NULL_FACTORY="$<TARGET_FILE:fixture_null_factory_module>"
        NEBULA_FIXTURE_INIT_FAIL="$<TARGET_FILE:fixture_init_fail_module>"
        NEBULA_FIXTURE_NO_TYPES="$<TARGET_FILE:fixture_no_types_module>"
        NEBULA_FIXTURE_NO_NODES="$<TARGET_FILE:fixture_no_nodes_module>"
        NEBULA_FIXTURE_WRONG_ID="$<TARGET_FILE:fixture_wrong_id_module>"
    )

    # The Modules needs to know where their shared libraries are
    target_compile_definitions(unit_tests PRIVATE
        NEBULA_C_MODULE="$<TARGET_FILE:neb_c_module>"
    )

    # The fixtures must exist before the tests run (and before CTest discovers
    # them).
    add_dependencies(unit_tests ${NEBULA_FIXTURE_TARGETS})

    # Auto-discover all Google Test tests
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
