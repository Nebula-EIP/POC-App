# CMakeLists.txt for core

add_library(neb_graph
    graph/graph.cpp
    graph/node.cpp
)

target_include_directories(neb_graph PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(core_capabilities
    modules/capabilities/type_list_capability.cpp
    modules/capabilities/node_list_capability.cpp
    modules/capabilities/renderer_capability.cpp
    modules/capabilities/importer_capability.cpp
)

target_include_directories(core_capabilities PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Capabilities are linked into the test module fixtures (shared libraries), so
# they must be position independent.
set_target_properties(core_capabilities PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Module loading. Kept separate from the GUI and from any concrete module.
add_library(core_modules
    modules/loader.cpp
    modules/shared_library.cpp
)

target_include_directories(core_modules PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_modules PUBLIC
    core_capabilities
    ${CMAKE_DL_LIBS}
)

set_target_properties(core_modules PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

add_library(core_node_catalog
    node_catalog/node_catalog.cpp
)

target_include_directories(core_node_catalog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_node_catalog PUBLIC
    core_modules
)

set_target_properties(core_node_catalog PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
