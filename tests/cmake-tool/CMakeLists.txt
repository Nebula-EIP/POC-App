# CMake Tool Tests

add_executable(cmake_tool_tests
    cmake_compiler_tests.cpp
)

target_link_libraries(cmake_tool_tests
    PRIVATE
        cmake_tool
        gtest
        gtest_main
        gmock
)

target_include_directories(cmake_tool_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/cmake-tool
)

# Add test discovery
include(GoogleTest)
gtest_discover_tests(cmake_tool_tests)
