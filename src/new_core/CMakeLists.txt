# CMakeLists.txt for new_core

add_library(new_core_capabilities
    modules/capabilities/type_list_capability.cpp
    modules/capabilities/node_list_capability.cpp
)

target_include_directories(new_core_capabilities PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
