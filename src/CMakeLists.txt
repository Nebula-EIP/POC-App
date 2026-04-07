# Source directory CMakeLists.txt

# Add subdirectories for each module
# Uncomment when modules are implemented
add_subdirectory(core)
add_subdirectory(utils)
# add_subdirectory(graph-builder)
# add_subdirectory(graph-exporter)

# CMake tool for single file compilation
add_subdirectory(cmake-tool)

# Main editor executable
add_subdirectory(editor)
