# Nebula Editor Executable

# Collect all source files
file(GLOB_RECURSE EDITOR_SOURCES "*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS "*.hpp" "*.h")

# Create editor executable
if(EDITOR_SOURCES)
    add_executable(Nebula ${EDITOR_SOURCES} ${EDITOR_HEADERS})

    target_compile_features(Nebula PRIVATE cxx_std_20)

    target_include_directories(Nebula
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    # Link with all project libraries when available
    # target_link_libraries(Nebula PRIVATE
    #     core
    #     utils
    #     graph-builder
    #     graph-exporter
    # )
else()
    message(FATAL_ERROR "No source files found in editor!")
endif()
