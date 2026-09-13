# Graph Exceptions

This document explains the exceptions related to graphs, nodes,
connections, and graph types.

## `GraphException`

Base class for all graph-related exceptions.

Use it when the error belongs to the graph system but does not have a
more specific exception.

``` cpp
throw core::GraphException("An unexpected graph error occurred.");
```

Prefer a more specific exception when possible.

------------------------------------------------------------------------

# Node Exceptions

## `NodeException`

Base class for errors specifically related to nodes.

Use it when the error concerns a node but does not fit a more specific
node exception.

------------------------------------------------------------------------

## `NodeNotFoundException`

Use when a requested node does not exist in the graph.

Examples: - Looking up an unknown `NodeId`. - Removing a node that does
not exist. - Trying to access a node that was already removed.

``` cpp
throw core::NodeNotFoundException("Node 42 was not found.");
```

------------------------------------------------------------------------

## `InvalidNodeException`

Use when a node exists or is being created, but its configuration or
data is invalid.

Examples: - Invalid node data. - Invalid node type. - Invalid node
properties. - A node contains invalid pin information.

Use `NodeNotFoundException` instead when the problem is simply that the
node does not exist.

------------------------------------------------------------------------

## `NodeAlreadyExistsException`

Use when creating or registering a node that conflicts with an existing
node.

Example:

``` cpp
throw core::NodeAlreadyExistsException(
    "A node with this id already exists."
);
```

------------------------------------------------------------------------

## `NodeConfigurationException`

Use when the configuration provided by a node type is invalid.

Examples: - Invalid input or output pin configuration. - Invalid default
properties. - A configuration does not match the node type.

This is especially useful when validating a `NodeConfiguration` provided
by a module capability.

------------------------------------------------------------------------

# Connection Exceptions

## `ConnectionException`

Base class for errors related to graph connections.

Use it when the error concerns a connection but does not fit a more
specific connection exception.

------------------------------------------------------------------------

## `InvalidConnectionException`

Use when a requested connection is structurally invalid.

Examples: - Connecting an input to another input. - Connecting an output
to another output. - Using an invalid pin. - Connecting nodes that
cannot be connected.

``` cpp
throw core::InvalidConnectionException(
    "The selected pins cannot be connected."
);
```

------------------------------------------------------------------------

## `ConnectionAlreadyExistsException`

Use when a connection already exists and the operation tries to create
it again.

Example:

``` cpp
throw core::ConnectionAlreadyExistsException(
    "This connection already exists."
);
```

------------------------------------------------------------------------

## `ConnectionNotFoundException`

Use when trying to remove or modify a connection that does not exist.

Example:

``` cpp
throw core::ConnectionNotFoundException(
    "The requested connection was not found."
);
```

------------------------------------------------------------------------

# Type Exceptions

## `TypeException`

Base class for errors related to graph data types.

Use it when the error concerns a type but does not fit a more specific
type exception.

------------------------------------------------------------------------

## `TypeNotFoundException`

Use when a required `DataType` does not exist.

Examples: - A node references an unknown type. - A graph operation
requests an unknown type.

------------------------------------------------------------------------

## `TypeAlreadyExistsException`

Use when registering a type that conflicts with an existing type.

This can happen when type identifiers or type names must be unique.

------------------------------------------------------------------------

## `TypeMismatchException`

Use when two types are not compatible for an operation.

The most common case is connecting pins with incompatible data types.

``` cpp
throw core::TypeMismatchException(
    "Cannot connect Float output to String input."
);
```

------------------------------------------------------------------------

## `CircularDependencyException`

Use when the graph must remain acyclic and an operation would create a
cycle.

Example:

``` text
A -> B -> C
     ^    |
     |____|
```

Only use this exception if the graph is designed to forbid cycles.

------------------------------------------------------------------------

## General rule

Choose the most specific exception available.

``` text
Node does not exist
    -> NodeNotFoundException

Node data is invalid
    -> InvalidNodeException

Connection does not exist
    -> ConnectionNotFoundException

Connection is invalid
    -> InvalidConnectionException

Types are incompatible
    -> TypeMismatchException

Operation would create a cycle
    -> CircularDependencyException
```
