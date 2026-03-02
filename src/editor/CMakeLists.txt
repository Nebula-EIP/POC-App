# Editor executable CMakeLists.txt

# Collect all source files (excluding main.cpp for now)
file(GLOB_RECURSE EDITOR_SOURCES
    "*.cpp"
)

file(GLOB_RECURSE EDITOR_HEADERS
    "*.hpp"
    "*.h"
)

# Create editor executable
if(EDITOR_SOURCES)
    add_executable(editor ${EDITOR_SOURCES} ${EDITOR_HEADERS})

    target_include_directories(editor
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    # Link with all project libraries
    # Uncomment when libraries are available
    # target_link_libraries(editor PRIVATE
    #     core
    #     utils
    #     graph-builder
    #     graph-exporter
    # )

    # Set properties
    set_target_properties(editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
else()
    message(STATUS "No source files found in editor - skipping executable creation")
endif()
