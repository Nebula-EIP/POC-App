add_executable(Nebula
    main.cpp
    application.cpp
    ui/component_adapter.cpp
)

target_include_directories(Nebula PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Nebula PRIVATE
    neb_graph
    core_modules
    utils
    raylib
    project_warnings
    neb_render
)
