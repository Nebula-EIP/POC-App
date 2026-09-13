# CMakeLists.txt for new_core

add_library(new_core_capabilities
    modules/capabilities/type_list_capability.cpp
    modules/capabilities/node_list_capability.cpp
    modules/capabilities/renderer_capability.cpp
)

target_include_directories(new_core_capabilities PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Capabilities are linked into the test module fixtures (shared libraries), so
# they must be position independent.
set_target_properties(new_core_capabilities PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Module loading. Kept separate from the GUI and from any concrete module.
add_library(new_core_modules
    modules/loader.cpp
    modules/shared_library.cpp
)

target_include_directories(new_core_modules PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(new_core_modules PUBLIC
    new_core_capabilities
    ${CMAKE_DL_LIBS}
)

set_target_properties(new_core_modules PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
