# Shared library fixtures used by the ModuleLoader tests.
#
# Every fixture is built from tests/core/modules/fixtures/fixture_module.cpp
# with a different set of FIXTURE_* definitions. They are MODULE libraries: they
# are never linked against, only dlopen'd / LoadLibrary'd by the tests.

set(NEBULA_FIXTURE_TARGETS "")

# fixture_module(<target> [<definition>...])
function(nebula_add_module_fixture target)
    add_library(${target} MODULE fixture_module.cpp)

    target_link_libraries(${target} PRIVATE core_capabilities)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src/core)
    target_compile_definitions(${target} PRIVATE ${ARGN})

    # No "lib" prefix and no import library: the tests build the file name
    # themselves from the target file path, nothing links to these.
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        POSITION_INDEPENDENT_CODE ON
    )

    set(NEBULA_FIXTURE_TARGETS ${NEBULA_FIXTURE_TARGETS} ${target} PARENT_SCOPE)
endfunction()

# A complete, valid module: both mandatory capabilities, no optional one.
nebula_add_module_fixture(fixture_valid_module
    FIXTURE_MODULE_NAME=\"ValidFixture\"
)

# A second valid module, used to check ids, ordering and aggregation.
nebula_add_module_fixture(fixture_second_module
    FIXTURE_MODULE_NAME=\"SecondFixture\"
)

# A valid module that also provides the optional renderer capability.
nebula_add_module_fixture(fixture_renderer_module
    FIXTURE_MODULE_NAME=\"RendererFixture\"
    FIXTURE_PROVIDE_RENDERER=1
)

# A library that exports no CreateModule symbol.
nebula_add_module_fixture(fixture_no_factory_module
    FIXTURE_NO_FACTORY=1
)

# CreateModule returns nullptr.
nebula_add_module_fixture(fixture_null_factory_module
    FIXTURE_NULL_FACTORY=1
)

# initialize() returns false.
nebula_add_module_fixture(fixture_init_fail_module
    FIXTURE_MODULE_NAME=\"InitFailFixture\"
    FIXTURE_INITIALIZE_RESULT=false
)

# Mandatory type list capability missing.
nebula_add_module_fixture(fixture_no_types_module
    FIXTURE_MODULE_NAME=\"NoTypesFixture\"
    FIXTURE_PROVIDE_TYPES=0
)

# Mandatory node list capability missing.
nebula_add_module_fixture(fixture_no_nodes_module
    FIXTURE_MODULE_NAME=\"NoNodesFixture\"
    FIXTURE_PROVIDE_NODES=0
)

# Reports an id that is not the one the loader assigned.
nebula_add_module_fixture(fixture_wrong_id_module
    FIXTURE_MODULE_NAME=\"WrongIdFixture\"
    FIXTURE_WRONG_ID=1
)

set(NEBULA_FIXTURE_TARGETS ${NEBULA_FIXTURE_TARGETS} PARENT_SCOPE)
