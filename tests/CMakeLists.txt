# Core tests
add_executable(nebula_tests
    main_test.cpp
)

target_link_libraries(nebula_tests PRIVATE gtest gtest_main)
target_compile_features(nebula_tests PRIVATE cxx_std_20)

# Register tests with CTest
add_test(NAME NebulaCoreTests COMMAND nebula_tests)
