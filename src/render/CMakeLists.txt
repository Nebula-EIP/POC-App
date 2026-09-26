# CMakeLists.txt for render
#
# Deliberately independent of core: this library knows nothing about
# core::Graph/Node. It exposes Renderer/Camera and takes graph content as
# opaque draw callbacks (see FrameCallbacks in renderer.hpp) — the editor is
# what wires core::Graph into these callbacks.

add_library(neb_render
    camera.cpp
    renderer.cpp
)

target_include_directories(neb_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(neb_render
    PUBLIC
        raylib
        utils
    PRIVATE
        project_warnings
)