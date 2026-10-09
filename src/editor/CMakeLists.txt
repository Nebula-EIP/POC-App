add_executable(Nebula
    main.cpp
    application.cpp
    ui/component_adapter.cpp
    ui/connection_request.cpp
    ui/node_view.cpp
    ui/node_canvas.cpp
    ui/notification_stack.cpp
    ui/top_bar.cpp
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
