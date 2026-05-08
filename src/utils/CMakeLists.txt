# Utils library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE UTILS_SOURCES
    "*.cpp"
)

file(GLOB_RECURSE UTILS_HEADERS
    "*.hpp"
    "*.h"
)

# Create utils library
if(UTILS_SOURCES)
    add_library(utils ${UTILS_SOURCES} ${UTILS_HEADERS})

    target_include_directories(utils
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )

    # Link with core if needed
    # target_link_libraries(utils PUBLIC core)

    # Find Raylib
    find_package(raylib REQUIRED)

    # Add dependencies
    target_link_libraries(core PUBLIC
        nlohmann_json::nlohmann_json
        utils
        raylib
    )

    if(UNIX AND NOT APPLE)
        target_link_libraries(core PUBLIC m dl pthread GL X11)
    elseif(WIN32)
        target_link_libraries(core PUBLIC
            winmm        # audio
            gdi32        # graphics
            opengl32     # OpenGL
        )
    endif()

    # Set properties
    set_target_properties(utils PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    message(STATUS "No source files found in utils - skipping library creation")
endif()
