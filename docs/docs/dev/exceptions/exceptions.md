# Core Exceptions

This document explains the general exceptions used by the core.

## `Exception`

Base class for all core exceptions.

Use it when an error belongs to the core but does not have a more
specific exception.

``` cpp
throw core::Exception("An unexpected core error occurred.");
```

Prefer a more specific exception when the error can be clearly
identified.

------------------------------------------------------------------------

## `InvalidArgumentException`

Use when a function receives an invalid argument.

Examples: - A negative value where only positive values are allowed. -
An invalid configuration value. - An argument that does not meet the
function requirements.

``` cpp
throw core::InvalidArgumentException("The node id must not be zero.");
```

Do not use it when the problem is caused by the current state of an
object.

------------------------------------------------------------------------

## `InvalidStateException`

Use when an object is in a state that does not allow the requested
operation.

Examples: - Trying to modify a graph while it is locked. - Trying to
perform an operation after shutdown. - Trying to remove something while
another operation requires it to stay available.

``` cpp
throw core::InvalidStateException("The graph is currently locked.");
```

------------------------------------------------------------------------

## `NotInitializedException`

Use when an operation requires an object or system to be initialized
first.

Example:

``` cpp
throw core::NotInitializedException("The module loader is not initialized.");
```

Use this when initialization is a required precondition.

------------------------------------------------------------------------

## `AlreadyInitializedException`

Use when an initialization operation is called on an object that is
already initialized.

Example:

``` cpp
throw core::AlreadyInitializedException("The module is already initialized.");
```

------------------------------------------------------------------------

## `NotFoundException`

Use for a generic resource that cannot be found when the resource is
required.

Examples: - A required resource is missing. - A generic object cannot be
found.

Prefer a more specific exception such as `NodeNotFoundException` or
`ModuleNotFoundException` when possible.

------------------------------------------------------------------------

## `AlreadyExistsException`

Use when a generic resource cannot be created or registered because it
already exists.

Prefer a more specific exception when the resource has a specific
domain, such as a node or module.

------------------------------------------------------------------------

## `OperationNotSupportedException`

Use when the requested operation is valid in general but is not
supported by the current object or implementation.

Example:

``` cpp
throw core::OperationNotSupportedException(
    "This module does not support this operation."
);
```

------------------------------------------------------------------------

## General rule

Use the most specific exception that describes the error.

For example:

``` text
Node does not exist
    -> NodeNotFoundException

Module does not exist
    -> ModuleNotFoundException

Invalid function argument
    -> InvalidArgumentException

Object is in the wrong state
    -> InvalidStateException
```
