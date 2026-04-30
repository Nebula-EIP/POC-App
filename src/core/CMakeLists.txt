# Core library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE CORE_SOURCES
    "*.cpp"
)

file(GLOB_RECURSE CORE_HEADERS
    "*.hpp"
    "*.h"
)

# Create core library
if(CORE_SOURCES)
    add_library(core ${CORE_SOURCES} ${CORE_HEADERS})

    target_include_directories(core
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )

    # Find Raylib
    find_package(raylib REQUIRED)

    # Add dependencies
    target_link_libraries(core PUBLIC
        nlohmann_json::nlohmann_json
        utils
        raylib
    )

    # Linux-specific system libs raylib depends on
    if(UNIX AND NOT APPLE)
        target_link_libraries(core PUBLIC m dl pthread GL X11)
    endif()

    # Set properties
    set_target_properties(core PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    message(STATUS "No source files found in core - skipping library creation")
endif()
