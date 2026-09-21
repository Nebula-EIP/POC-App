add_library(editor_ui
    ui/component_adapter.cpp
    ui/component_adapter.hpp
)

target_include_directories(editor_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(editor_ui PUBLIC
    project_warnings
    core_capabilities
    utils
)
