# CMakeLists.txt for neb_c_module

add_library(neb_c_module SHARED
    # Add module source files here when implementation begins.
    create_module.cpp
    module.cpp
    types_list.cpp
    nodes_list.cpp
    renderer/renderer.cpp
    renderer/factory.cpp
)

target_link_libraries(neb_c_module
    PRIVATE
        core_modules
)

target_include_directories(neb_c_module
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Inform that the lib data can be safely copied over
set_target_properties(neb_c_module PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set_target_properties(neb_c_module PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules" # Linux
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules" # Windows
)
