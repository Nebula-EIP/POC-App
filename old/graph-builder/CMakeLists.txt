# Graph-builder library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE GRAPH_BUILDER_SOURCES
    "*.cpp"
)

file(GLOB_RECURSE GRAPH_BUILDER_HEADERS
    "*.hpp"
    "*.h"
)

# Create graph-builder library
if(GRAPH_BUILDER_SOURCES)
    add_library(graph-builder ${GRAPH_BUILDER_SOURCES} ${GRAPH_BUILDER_HEADERS})

    target_include_directories(graph-builder
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )

    # Link with dependencies
    # target_link_libraries(graph-builder PUBLIC core utils)

    # Set properties
    set_target_properties(graph-builder PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    message(STATUS "No source files found in graph-builder - skipping library creation")
endif()
