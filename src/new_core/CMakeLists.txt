add_library(new_core_capabilities
    modules/capabilities/renderer_capability.cpp
)

target_include_directories(new_core_capabilities
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
