# Example executables

set(EDITOR_CODEGEN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/code-generation/code-generator-file-formatting.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/code-generation/codegen-context.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/code-generation/codegen-optimizer.cpp
    ${CMAKE_SOURCE_DIR}/src/editor/file-writing/file-writer.cpp
)

function(add_example_target target_name source_file)
    add_executable(${target_name} ${source_file})
    target_sources(${target_name} PRIVATE ${EDITOR_CODEGEN_SOURCES})
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/editor
    )
    target_link_libraries(${target_name} PRIVATE core utils graph-exporter)
endfunction()

add_example_target(codegen_demo codegen_demo.cpp)

