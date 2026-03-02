# Tests CMakeLists.txt

# Enable testing
enable_testing()

# Find or fetch a testing framework (example with Google Test)
# Uncomment when you want to add tests
# find_package(GTest)
# if(NOT GTest_FOUND)
#     include(FetchContent)
#     FetchContent_Declare(
#         googletest
#         GIT_REPOSITORY https://github.com/google/googletest.git
#         GIT_TAG release-1.12.1
#     )
#     FetchContent_MakeAvailable(googletest)
# endif()

# Collect test files
file(GLOB_RECURSE TEST_SOURCES
    "*.cpp"
)

# Create test executables
if(TEST_SOURCES)
    foreach(TEST_FILE ${TEST_SOURCES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_FILE})

        # Link with project libraries and testing framework
        # target_link_libraries(${TEST_NAME} PRIVATE
        #     core
        #     utils
        #     GTest::gtest_main
        # )

        # Add test to CTest
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
else()
    message(STATUS "No test files found - skipping test creation")
endif()
