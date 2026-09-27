add_executable(Nebula
    main.cpp
    application.cpp
    ui/component_adapter.cpp
    ui/node_view.cpp
    ui/node_canvas.cpp
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
)
