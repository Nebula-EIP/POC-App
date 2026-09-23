# CMakeLists.txt for editor

# raylib is provided by the CPMAddPackage(NAME raylib ...) call in the root
# CMakeLists.txt, evaluated before add_subdirectory(src) — no find_package
# needed here, the "raylib" target already exists.

add_executable(Nebula
    main.cpp
    application.cpp
)

target_include_directories(Nebula PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Nebula PRIVATE
    neb_graph
    core_modules
    raylib
    project_warnings
)

# Output directory is inherited from CMAKE_RUNTIME_OUTPUT_DIRECTORY, set
# globally in the root CMakeLists.txt — nothing to configure here.