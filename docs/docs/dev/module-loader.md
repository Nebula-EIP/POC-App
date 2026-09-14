---
sidebar_position: 3
---

# Module Loader

`core::ModuleLoader` owns every loaded module and the shared library each one came
from. It is implemented in `src/core/modules/loader.hpp`,
`src/core/modules/loader.tcc` and `src/core/modules/loader.cpp`, on top of
the small platform wrapper in `src/core/modules/shared_library.hpp`.

The loader depends only on the module capabilities and the standard library: it
knows nothing about the C module or the editor, and can be used without a window.

## Loading a module

```cpp
core::ModuleLoader loader;

const core::ModuleId id = loader.Load("modules/my_module.so");
core::IModule *module = loader.Module(id);
```

`Load()` performs the following steps, in order:

1. the path must exist and be a regular file;
2. the same file must not already be loaded;
3. the library is opened (`dlopen` on Linux, `LoadLibraryExW` on Windows);
4. the `CreateModule` symbol is resolved and called;
5. the returned module must not be null, and its name must not already be used by
   a loaded module;
6. `Initialize(id)` is called with a fresh nonzero id, and must return `true`;
7. the module must report that same id through `Id()`;
8. the two mandatory capabilities, `Types()` and `Nodes()`, must be available.

Optional capabilities (renderer, exporter, importer, ...) are never required. A
module that provides none loads normally, and `Capability<T>()` simply returns
`nullptr` for them.

Ids are handed out monotonically from 1 and are **never reused**, so a stale id can
never designate a different module. A failed load consumes nothing: the rejected
id is given to the next call.

## Querying loaded modules

```cpp
core::IModule *by_id = loader.Module(id);
core::IModule *by_name = loader.Module("MyModule");

for (const core::IModule *module : loader.Modules()) {
    // in load order
}
```

`Capabilities<T>()` collects one capability type across every loaded module, which
is how the rest of the core reaches module provided data without knowing which
module provides what:

```cpp
for (const core::capa::ITypeListCapability *types :
     loader.Capabilities<core::capa::ITypeListCapability>()) {
    // every module exports its types here
}
```

Modules that do not provide the capability are skipped, so an empty span means no
loaded module provides it. The two mandatory capabilities are resolved through
`Types()` and `Nodes()` as well, so a module does not have to register them in its
`Capability(std::type_index)` map.

## Error reporting convention

There are exactly two conventions, and they never mix.

| Operation | On failure |
| --- | --- |
| `Load()` | throws, always something derived from `core::ModuleException` |
| `Module()` | returns `nullptr` |
| `Unload()` | returns `false` |
| `Modules()`, `Capabilities()` | return an empty view |

`Load()` is therefore the only operation that throws, and it never returns 0: the
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
| `Initialize()` returned `false` | `ModuleInitializationException` |

Every message ends with the path of the module being loaded, and the operating
system error is appended when the platform provided one.

## Unloading and cleanup

```cpp
loader.Unload(id);            // by id
loader.Unload("MyModule");    // by name
loader.UnloadAll();           // everything, in reverse load order
```

A module is always destroyed **before** its library is closed:

```
Shutdown()  ->  ~IModule()  ->  dlclose() / FreeLibrary()
```

`Shutdown()` is only called on a module whose `Initialize()` returned `true`. The
loader destructor runs `UnloadAll()`, so a loader going out of scope cleans up
every module it still owns.

A failed load releases everything it created, in that same order, and leaves the
loader exactly as it was, which makes the loader usable again immediately after a
rejected module.

## Lifetimes

- Pointers from `Module()` are valid until that module is unloaded or the loader is
  destroyed. They are not owning: never `delete` them.
- Views from `Modules()` and `Capabilities<T>()` are invalidated by any `Load()`,
  `Unload()` or `UnloadAll()`, by the next `Capabilities<T>()` call for the same
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

- return a non empty, unique name from `Name()`;
- store the id given to `Initialize()` and return it from `Id()`;
- provide both `Types()` and `Nodes()`;
- release everything it owns in `Shutdown()`, which is called before destruction.

The instance is allocated by the library and destroyed by the core through the
`IModule` virtual destructor, so the library must be built against the same C++
runtime as the core.

`ModuleId` is defined once, in `src/core/graph/datatypes.hpp`. Do not
redeclare it.

## Tests

`tests/core/modules/loader_test.cpp` covers the loader, backed by the shared
library fixtures in `tests/core/modules/fixtures/`. All fixtures are built from
a single source with different `FIXTURE_*` definitions, they are `MODULE` libraries
that nothing links against, and they open no window.

```sh
ctest --test-dir build -R ModuleLoaderTest --output-on-failure
```

Adding a new fixture means one `nebula_add_module_fixture()` call in
`tests/core/modules/fixtures/CMakeLists.txt` and one
`NEBULA_FIXTURE_*` definition in `tests/CMakeLists.txt`, which passes its path to
the tests.
