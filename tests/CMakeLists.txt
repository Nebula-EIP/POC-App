# Tests CMakeLists.txt

# Collect all test files
file(GLOB TEST_SOURCES
    "main_test.cpp"
    "core/*.cpp"
    "cmake-tool/*.cpp"
)

# Create test executable
if(TEST_SOURCES)
    add_executable(nebula_tests ${TEST_SOURCES})

    target_link_libraries(nebula_tests
        PRIVATE
            core
            cmake_tool
            gtest
            gtest_main
            gmock
    )

    target_compile_features(nebula_tests PRIVATE cxx_std_23)

    target_include_directories(nebula_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/cmake-tool
    )

    # Add test discovery
    include(GoogleTest)
    gtest_discover_tests(nebula_tests)
else()
    message(STATUS "No test files found - skipping test creation")
endif()
