# Tests CMakeLists.txt

# Shared library fixtures loaded at runtime by the ModuleLoader tests.
# Added first so that NEBULA_FIXTURE_TARGETS is available below.
add_subdirectory(new_core/modules/fixtures)

# Collect test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# The module fixtures are shared libraries of their own, they must never be
# compiled into the test executable.
list(FILTER TEST_SOURCES EXCLUDE REGEX "/new_core/modules/fixtures/")

# Create test executable
if(TEST_SOURCES)
    add_executable(unit_tests ${TEST_SOURCES})

    # Code-generation lives under src/editor and isn't a standalone library yet.
    # Compile the code-generation sources into the test executable.
    target_sources(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file-formatting.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/codegen-context.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/code-generation/codegen-optimizer.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/file-writing/file-writer.cpp
        ${CMAKE_SOURCE_DIR}/src/editor/ui/top_bar.cpp
    )

    # Allow includes like "code-generation/..." and "errors/...".
    target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor
        ${CMAKE_SOURCE_DIR}/src/new_core
        ${CMAKE_SOURCE_DIR}/examples
    )

    # code-generator-file-formatting depends on utils/string-splitter.hpp
    target_link_libraries(unit_tests PRIVATE utils)

    # Link with project libraries when available
    target_link_libraries(unit_tests PRIVATE
        gtest
        gtest_main
        gmock
        core
        new_core_capabilities
        new_core_modules
        graph-exporter
        cmake_tool
        #utils
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

    # The fixtures must exist before the tests run (and before CTest discovers
    # them).
    add_dependencies(unit_tests ${NEBULA_FIXTURE_TARGETS})

    # Auto-discover all Google Test tests
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
