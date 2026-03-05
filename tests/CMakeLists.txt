# Tests CMakeLists.txt

# Collect test files for main tests (excluding subdirectories)
file(GLOB TEST_SOURCES
    "main_test.cpp"
    "core/*.cpp"
    "cmake-tool/*.cpp"
)

# Collect cmake-tool test files
file(GLOB CMAKE_TOOL_TEST_SOURCES
    "cmake-tool/*.cpp"
)

# Create main test executable
if(TEST_SOURCES)
    add_executable(nebula_tests ${TEST_SOURCES})

    target_link_libraries(nebula_tests PRIVATE gtest gtest_main)
    target_compile_features(nebula_tests PRIVATE cxx_std_23)

    # Link with project libraries when available
    target_link_libraries(nebula_tests PRIVATE
        core
        #utils
    )

    # Register tests with CTest
    add_test(NAME NebulaCoreTests COMMAND nebula_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()

# Create cmake-tool test executable
if(CMAKE_TOOL_TEST_SOURCES)
    add_executable(cmake_tool_tests ${CMAKE_TOOL_TEST_SOURCES})

    target_link_libraries(cmake_tool_tests
        PRIVATE
            cmake_tool
            gtest
            gtest_main
            gmock
    )

    target_compile_features(cmake_tool_tests PRIVATE cxx_std_23)

    target_include_directories(cmake_tool_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/cmake-tool
    )

    # Add test discovery
    include(GoogleTest)
    gtest_discover_tests(cmake_tool_tests)
else()
    message(STATUS "No cmake-tool test files found")
endif()
