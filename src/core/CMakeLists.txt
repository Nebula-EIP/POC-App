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

    # Add dependencies here when needed
    # target_link_libraries(core PUBLIC ...)

    # Set properties
    set_target_properties(core PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    message(STATUS "No source files found in core - skipping library creation")
endif()
