---
sidebar_position: 3
---

# Module Loader

`core::ModuleLoader` owns every loaded module and the shared library each one came
from. It is implemented in `src/new_core/modules/loader.hpp`,
`src/new_core/modules/loader.tcc` and `src/new_core/modules/loader.cpp`, on top of
the small platform wrapper in `src/new_core/modules/shared_library.hpp`.

The loader depends only on the module capabilities and the standard library: it
knows nothing about the C module or the editor, and can be used without a window.

## Loading a module

```cpp
core::ModuleLoader loader;

const core::ModuleId id = loader.load("modules/my_module.so");
core::IModule *module = loader.module(id);
```

`load()` performs the following steps, in order:

1. the path must exist and be a regular file;
2. the same file must not already be loaded;
3. the library is opened (`dlopen` on Linux, `LoadLibraryExW` on Windows);
4. the `CreateModule` symbol is resolved and called;
5. the returned module must not be null, and its name must not already be used by
   a loaded module;
6. `initialize(id)` is called with a fresh nonzero id, and must return `true`;
7. the module must report that same id through `id()`;
8. the two mandatory capabilities, `types()` and `nodes()`, must be available.

Optional capabilities (renderer, exporter, importer, ...) are never required. A
module that provides none loads normally, and `capability<T>()` simply returns
`nullptr` for them.

Ids are handed out monotonically from 1 and are **never reused**, so a stale id can
never designate a different module. A failed load consumes nothing: the rejected
id is given to the next call.

## Querying loaded modules

```cpp
core::IModule *by_id = loader.module(id);
core::IModule *by_name = loader.module("MyModule");

for (const core::IModule *module : loader.modules()) {
    // in load order
}
```

`capabilities<T>()` collects one capability type across every loaded module, which
is how the rest of the core reaches module provided data without knowing which
module provides what:

```cpp
for (const core::capa::ITypeListCapability *types :
     loader.capabilities<core::capa::ITypeListCapability>()) {
    // every module exports its types here
}
```

Modules that do not provide the capability are skipped, so an empty span means no
loaded module provides it. The two mandatory capabilities are resolved through
`types()` and `nodes()` as well, so a module does not have to register them in its
`capability(std::type_index)` map.

## Error reporting convention

There are exactly two conventions, and they never mix.

| Operation | On failure |
| --- | --- |
| `load()` | throws, always something derived from `core::ModuleException` |
| `module()` | returns `nullptr` |
| `unload()` | returns `false` |
| `modules()`, `capabilities()` | return an empty view |

`load()` is therefore the only operation that throws, and it never returns 0: the
id it returns is always valid. The exception names the exact problem, following
the [module exceptions](./exceptions/module_exception.md) catalogue:

| Failure | Exception |
| --- | --- |
| empty path, missing file, not a regular file | `ModuleFileNotFoundException` |
| the OS refused the library (wrong format, missing dependency) | `ModuleLoadFailedException` |
| no `CreateModule` symbol | `ModuleSymbolNotFoundException` |
| `CreateModule` returned `nullptr` | `InvalidModuleException` |
| empty name, wrong reported id, missing mandatory capability | `InvalidModuleException` |
| same file, or same module name, already loaded | `ModuleAlreadyLoadedException` |
| `initialize()` returned `false` | `ModuleInitializationException` |

Every message ends with the path of the module being loaded, and the operating
system error is appended when the platform provided one.

## Unloading and cleanup

```cpp
loader.unload(id);            // by id
loader.unload("MyModule");    // by name
loader.unloadAll();           // everything, in reverse load order
```

A module is always destroyed **before** its library is closed:

```
shutdown()  ->  ~IModule()  ->  dlclose() / FreeLibrary()
```

`shutdown()` is only called on a module whose `initialize()` returned `true`. The
loader destructor runs `unloadAll()`, so a loader going out of scope cleans up
every module it still owns.

A failed load releases everything it created, in that same order, and leaves the
loader exactly as it was, which makes the loader usable again immediately after a
rejected module.

## Lifetimes

- Pointers from `module()` are valid until that module is unloaded or the loader is
  destroyed. They are not owning: never `delete` them.
- Views from `modules()` and `capabilities<T>()` are invalidated by any `load()`,
  `unload()` or `unloadAll()`, by the next `capabilities<T>()` call for the same
  capability type, and by the destruction of the loader. Copy what you need out of
  them instead of storing them.
- The loader is not thread safe, and is neither copyable nor movable, because
  moving it would invalidate every view it handed out.

## Writing a loadable module

Include `modules/create_module.hpp` and export the factory with
`NEBULA_MODULE_EXPORT`, which is what makes the symbol visible on Windows:

```cpp
#include "modules/create_module.hpp"

extern "C" NEBULA_MODULE_EXPORT core::IModule *CreateModule() {
    return new MyModule();
}
```

A module must:

- return a non empty, unique name from `name()`;
- store the id given to `initialize()` and return it from `id()`;
- provide both `types()` and `nodes()`;
- release everything it owns in `shutdown()`, which is called before destruction.

The instance is allocated by the library and destroyed by the core through the
`IModule` virtual destructor, so the library must be built against the same C++
runtime as the core.

`ModuleId` is defined once, in `src/new_core/graph/datatypes.hpp`. Do not
redeclare it.

## Tests

`tests/new_core/modules/loader_test.cpp` covers the loader, backed by the shared
library fixtures in `tests/new_core/modules/fixtures/`. All fixtures are built from
a single source with different `FIXTURE_*` definitions, they are `MODULE` libraries
that nothing links against, and they open no window.

```sh
ctest --test-dir build -R ModuleLoaderTest --output-on-failure
```

Adding a new fixture means one `nebula_add_module_fixture()` call in
`tests/new_core/modules/fixtures/CMakeLists.txt` and one
`NEBULA_FIXTURE_*` definition in `tests/CMakeLists.txt`, which passes its path to
the tests.
