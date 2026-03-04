# Tests CMakeLists.txt

# Add subdirectories for specific module tests
add_subdirectory(cmake-tool)

# Collect test files for main tests (excluding subdirectories)
file(GLOB TEST_SOURCES 
    "main_test.cpp"
    "core/*.cpp"
)

# Create test executable
if(TEST_SOURCES)
    add_executable(nebula_tests ${TEST_SOURCES})

    target_link_libraries(nebula_tests PRIVATE gtest gtest_main)
    target_compile_features(nebula_tests PRIVATE cxx_std_20)

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
