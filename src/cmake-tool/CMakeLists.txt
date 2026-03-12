# CMake Tool Module - Library only (GUI integration)

add_library(cmake_tool STATIC
    cmake_compiler.cpp
)

target_include_directories(cmake_tool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
