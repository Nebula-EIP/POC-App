# Module Exceptions

This document explains the exceptions related to module loading,
validation, initialization, and capabilities.

## `ModuleException`

Base class for all module-related exceptions.

Use it when the error belongs to the module system but does not have a
more specific exception.

``` cpp
throw core::ModuleException("An unexpected module error occurred.");
```

Prefer a more specific exception when possible.

------------------------------------------------------------------------

# Module Loading

## `ModuleLoadException`

Use when a dynamic library cannot be loaded.

Examples: - The shared library cannot be opened. - The library is
invalid. - The operating system refuses to load the library.

``` cpp
throw core::ModuleLoadException(
    "Failed to load the module library."
);
```

------------------------------------------------------------------------

## `ModuleFileNotFoundException`

Use when the module file does not exist at the requested path.

``` cpp
throw core::ModuleFileNotFoundException(
    "Module file was not found."
);
```

Use this instead of the more general `ModuleLoadException` when the
exact problem is that the file is missing.

------------------------------------------------------------------------

## `ModuleLoadFailedException`

Use when the module file exists but loading it still fails.

Examples: - The library has invalid dependencies. - The library format
is invalid. - The operating system reports a loading error.

------------------------------------------------------------------------

## `ModuleSymbolNotFoundException`

Use when the library is loaded successfully but a required symbol cannot
be found.

This is particularly useful for the module factory:

``` cpp
extern "C" {
    core::IModule* CreateModule();
}
```

If `CreateModule` is missing:

``` cpp
throw core::ModuleSymbolNotFoundException(
    "CreateModule symbol was not found."
);
```

------------------------------------------------------------------------

# Module Validation

## `InvalidModuleException`

Use when a module was loaded but does not provide a valid `IModule`.

Examples: - `CreateModule()` returns `nullptr`. - Required module
information is invalid. - Required module capabilities are missing. -
The module does not follow the expected interface contract.

------------------------------------------------------------------------

## `IncompatibleModuleException`

Use when the module is valid by itself but cannot work with the current
core.

Examples: - The module requires an unsupported core feature. - The
module was built for an incompatible API.

------------------------------------------------------------------------

## `ModuleVersionException`

Use when the module and core use incompatible API versions.

Example:

``` text
Core API:    2.0
Module API:  1.0
```

Use this when version compatibility is specifically the reason the
module cannot be used.

------------------------------------------------------------------------

# Module State

## `ModuleInitializationException`

Use when a module fails during initialization.

For example, when:

``` cpp
module->initialize(id)
```

returns `false` and the loader needs to report the failure as an
exception.

``` cpp
throw core::ModuleInitializationException(
    "Failed to initialize module."
);
```

------------------------------------------------------------------------

## `ModuleNotFoundException`

Use when a requested module does not exist.

Examples: - Unloading an unknown module. - Requesting a module by id
when it is not loaded. - Requesting a module by name when it does not
exist.

If an API already uses `nullptr` or `false` to report a missing module,
keep that API behavior instead of throwing.

------------------------------------------------------------------------

## `ModuleAlreadyLoadedException`

Use when loading a module that is already loaded is forbidden.

Example:

``` cpp
throw core::ModuleAlreadyLoadedException(
    "The module is already loaded."
);
```

Only use this if the module system does not allow multiple instances of
the same module.

------------------------------------------------------------------------

## `ModuleStateException`

Use when a module operation is invalid because of the module's current
state.

Examples: - Initializing an already initialized module. - Shutting down
a module that was never initialized. - Performing an operation after the
module has been shut down.

------------------------------------------------------------------------

# Capability Exceptions

## `CapabilityException`

Base class for capability-related errors.

Use it when the error concerns a capability but does not fit a more
specific exception.

------------------------------------------------------------------------

## `CapabilityNotFoundException`

Use when a capability is required but is not provided by the module.

Do not use this automatically for optional capabilities.

For example, if this is valid:

``` cpp
module->capability<IExporterCapability>();
```

returning `nullptr`, then no exception is needed.

Use the exception only when the capability is a required part of the
operation.

------------------------------------------------------------------------

## `InvalidCapabilityException`

Use when a module provides a capability, but the capability does not
satisfy the expected contract.

Examples: - Invalid capability data. - Inconsistent capability
configuration. - A capability violates a required interface rule.

------------------------------------------------------------------------

## General rule

Choose the most specific exception available.

``` text
File does not exist
    -> ModuleFileNotFoundException

Library cannot be loaded
    -> ModuleLoadException / ModuleLoadFailedException

CreateModule is missing
    -> ModuleSymbolNotFoundException

Loaded module is invalid
    -> InvalidModuleException

Module API is incompatible
    -> ModuleVersionException / IncompatibleModuleException

Module initialization failed
    -> ModuleInitializationException

Required capability is missing
    -> CapabilityNotFoundException
```

## Important: do not throw for every normal failure

Some existing module APIs already use return values:

``` cpp
module() -> nullptr
unload() -> false
initialize() -> false
```

Keep those semantics when the failure is an expected result of the
operation.

Exceptions should be used for errors that need to interrupt normal
control flow or provide a stronger error signal.
