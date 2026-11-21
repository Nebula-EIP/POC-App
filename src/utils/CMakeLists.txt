# Utils library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE UTILS_SOURCES
    "*.cpp"
)

file(GLOB_RECURSE UTILS_HEADERS
    "*.hpp"
    "*.h"
)

# The palette file is compiled in as the built-in palette (palette.cpp).
# Editing it re-runs CMake, which regenerates palette_default.cpp.
set(NEBULA_PALETTE_FILE "${CMAKE_SOURCE_DIR}/palette.yaml")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${NEBULA_PALETTE_FILE}")
file(READ "${NEBULA_PALETTE_FILE}" NEBULA_PALETTE_YAML)
configure_file(palette_default.cpp.in
    "${CMAKE_CURRENT_BINARY_DIR}/palette_default.cpp" @ONLY)
list(APPEND UTILS_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/palette_default.cpp")

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

    # Find Raylib
    find_package(raylib REQUIRED)

    target_link_libraries(utils PUBLIC raylib PRIVATE yaml-cpp::yaml-cpp)

    # Set properties
    set_target_properties(utils PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    message(STATUS "No source files found in utils - skipping library creation")
endif()
