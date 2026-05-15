# .nebula File Format Specification

**Version:** 1.0
**Date:** March 4, 2026
**Status:** Draft

## Overview

The `.nebula` file format is a JSON-based format for saving and loading Nebula graph projects. It stores the complete state of a graph including metadata, nodes, and connections.

## File Structure

A `.nebula` file is a valid JSON document with the following top-level structure:

```json
{
  "metadata": { ... },
  "graph": { ... }
}
```

## Metadata Section

The `metadata` section contains project-level information.

### Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `project_name` | string | Yes | Human-readable name of the project |
| `version` | string | Yes | Format version (currently "1.0") |
| `author` | string | No | Creator/owner of the project |
| `created_at` | string | Yes | ISO 8601 timestamp of project creation |
| `modified_at` | string | Yes | ISO 8601 timestamp of last modification |

### Example

```json
{
  "metadata": {
    "project_name": "My First Graph",
    "version": "1.0",
    "author": "John Doe",
    "created_at": "2026-03-04T10:30:00Z",
    "modified_at": "2026-03-04T15:45:30Z"
  }
}
```

## Graph Section

The `graph` section contains the actual graph data structure.

### Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `next_id` | integer | Yes | Next available node ID (for ID generation) |
| `nodes` | array | Yes | Array of node objects |
| `connections` | array | Yes | Array of connection objects |

### Node Object

Each node object represents a single node in the graph.

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `id` | integer | Yes | Unique node identifier (unsigned 32-bit) |
| `kind` | string | Yes | Node type (see Node Kinds below) |
| Additional fields | varies | Depends on kind | Node-specific properties |

#### Node Kinds

| Kind String | Description | Additional Fields |
|-------------|-------------|-------------------|
| `Literal` | Literal value node | `type` (string), `name` (string) |
| `Variable` | Variable reference | TBD |
| `Function` | Function definition | TBD |
| `FunctionInput` | Function input parameter | TBD |
| `FunctionOutput` | Function output/return | TBD |
| `Operator` | Mathematical/logical operator | TBD |
| `Condition` | Conditional/branching node | TBD |
| `Loop` | Loop/iteration node | TBD |

#### Literal Node Fields

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | Yes | Data type: "Undefined", "Int", "Float", "Bool", "String", "Void" |
| `name` | string | Yes | Display name for the literal |

### Connection Object

Each connection object represents a link between two nodes.

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `source_node_id` | integer | Yes | ID of the source (output) node |
| `source_pin` | integer | Yes | Output pin index on source node (0-based) |
| `target_node_id` | integer | Yes | ID of the target (input) node |
| `target_pin` | integer | Yes | Input pin index on target node (0-based) |

### Example

```json
{
  "graph": {
    "next_id": 5,
    "nodes": [
      {
        "id": 1,
        "kind": "Literal",
        "type": "Int",
        "name": "Number 42"
      },
      {
        "id": 2,
        "kind": "Literal",
        "type": "Float",
        "name": "Pi"
      },
      {
        "id": 3,
        "kind": "Operator",
        "operator_type": "Add"
      }
    ],
    "connections": [
      {
        "source_node_id": 1,
        "source_pin": 0,
        "target_node_id": 3,
        "target_pin": 0
      },
      {
        "source_node_id": 2,
        "source_pin": 0,
        "target_node_id": 3,
        "target_pin": 1
      }
    ]
  }
}
```

## Complete Example

Here's a complete `.nebula` file example with a simple two-node graph:

```json
{
  "metadata": {
    "project_name": "Hello Nebula",
    "version": "1.0",
    "author": "Nebula Team",
    "created_at": "2026-03-04T10:00:00Z",
    "modified_at": "2026-03-04T10:00:00Z"
  },
  "graph": {
    "next_id": 3,
    "nodes": [
      {
        "id": 1,
        "kind": "Literal",
        "type": "Int",
        "name": "My Number"
      },
      {
        "id": 2,
        "kind": "Literal",
        "type": "String",
        "name": "My Text"
      }
    ],
    "connections": []
  }
}
```

## Validation Rules

### Required Validations

1. **File must be valid JSON** - Malformed JSON should be rejected
2. **Version compatibility** - Only version "1.0" is currently supported
3. **Node ID uniqueness** - All node IDs must be unique within the graph
4. **next_id correctness** - Must be greater than all existing node IDs
5. **Connection validity** - All referenced node IDs must exist in the nodes array
6. **Pin indices** - Must be valid for the respective node type
7. **Node kind validity** - Must be one of the recognized node kinds

### Data Type Mappings

C++ PinDataType enum to JSON string:

| C++ Enum | JSON String |
|----------|-------------|
| `kUndefined` | "Undefined" |
| `kInt` | "Int" |
| `kFloat` | "Float" |
| `kBool` | "Bool" |
| `kString` | "String" |
| `kVoid` | "Void" |

C++ NodeKind enum to JSON string:

| C++ Enum | JSON String |
|----------|-------------|
| `kLiteral` | "Literal" |
| `kVariable` | "Variable" |
| `kFunction` | "Function" |
| `kFunctionInput` | "FunctionInput" |
| `kFunctionOutput` | "FunctionOutput" |
| `kOperator` | "Operator" |
| `kCondition` | "Condition" |
| `kLoop` | "Loop" |

## Error Handling

When loading a `.nebula` file, implementations should provide clear error messages for:

- Invalid JSON syntax
- Missing required fields
- Invalid data types
- Version mismatch
- Unknown node kinds
- Dangling connections (referencing non-existent nodes)
- Invalid pin indices
- ID collisions

## Future Considerations

Version 1.0 is intentionally minimal. Future versions may include:

- Node position/layout data for visual editor
- Custom node types and plugins
- Embedded resources (images, files)
- Undo/redo history
- Graph metadata (description, tags, thumbnail)
- Compression support
- Incremental save format

## File Extension

The recommended file extension is `.nebula`.

## MIME Type

Recommended MIME type: `application/x-nebula+json`

## Character Encoding

All `.nebula` files must use UTF-8 encoding.
