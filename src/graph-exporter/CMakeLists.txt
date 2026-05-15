# Graph-exporter library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE GRAPH_EXPORTER_SOURCES
    "*.cpp"
)

file(GLOB_RECURSE GRAPH_EXPORTER_HEADERS
    "*.hpp"
    "*.h"
)

# Create graph-exporter library
if(GRAPH_EXPORTER_SOURCES)
    add_library(graph-exporter ${GRAPH_EXPORTER_SOURCES} ${GRAPH_EXPORTER_HEADERS})

    target_include_directories(graph-exporter
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )

    # Link with dependencies
    # target_link_libraries(graph-exporter PUBLIC core utils)

    # Set properties
    set_target_properties(graph-exporter PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    message(STATUS "No source files found in graph-exporter - skipping library creation")
endif()
