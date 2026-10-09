# CMakeLists.txt for render

add_library(neb_render
    camera.cpp
    grid.cpp
    renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/ui/node_canvas.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/ui/node_view.cpp
)

target_include_directories(neb_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/editor
)

target_link_libraries(neb_render
    PUBLIC
        raylib
        utils
        neb_graph
    PRIVATE
        project_warnings
)