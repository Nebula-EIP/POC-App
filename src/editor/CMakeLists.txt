add_executable(Nebula
    main.cpp
    application.cpp
    configuration.cpp
    ui/connection_request.cpp
    ui/node_view.cpp
    ui/node_canvas.cpp
    ui/node_clipboard.cpp
    ui/notification_stack.cpp
    ui/top_bar.cpp
    ui/context_menu.cpp
    ui/node_picker.cpp
    actions/action.cpp
    actions/action_manager.cpp
)

target_include_directories(Nebula PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Nebula PRIVATE
    neb_graph
    core_modules
    core_node_catalog
    utils
    raylib
    project_warnings
    neb_render
)
