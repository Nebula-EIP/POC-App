# Tests CMakeLists.txt

# Collect test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Create test executable
if(TEST_SOURCES)
    add_executable(nebula_tests ${TEST_SOURCES})

    target_link_libraries(nebula_tests PRIVATE gtest gtest_main gmock)
    target_compile_features(nebula_tests PRIVATE cxx_std_20)

    # Link with project libraries when available
    target_link_libraries(nebula_tests PRIVATE
        core
        cmake_tool
        #utils
    )

    target_include_directories(nebula_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/cmake-tool)

    # Register tests with CTest
    add_test(NAME NebulaCoreTests COMMAND nebula_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
