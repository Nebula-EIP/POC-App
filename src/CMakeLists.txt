# Source directory CMakeLists.txt

# Add subdirectories for each module
add_subdirectory(utils)
add_subdirectory(core)

add_subdirectory(editor)

# Main editor executable
# add_subdirectory(editor)  # DISABLED: depends on old core library
