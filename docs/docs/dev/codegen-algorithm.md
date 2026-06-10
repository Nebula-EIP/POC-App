---
sidebar_position: 2
---

# C++ Code Generation Pipeline

This page describes the algorithm used to transform a Nebula graph into generated C++ code.

## Goal

The goal is straightforward: take a connected node graph, validate it, order its dependencies, and produce a compilable C++ file.

The generator mainly aims to do two things:

1. produce correct, compilable code
2. keep the output readable for demos and human-friendly exports

## Overview

The current pipeline follows these steps:

1. graph validation
2. topological sorting of nodes
3. emission of C++ expressions and statements
4. special handling for control blocks (`if`, `else`, `while`, `for`)
5. optional constant folding
6. export to a `.cpp` file

## 1. Graph Validation

Before generation starts, the graph is validated by `GraphValidator`.

### What this phase checks

- dependency cycles
- type compatibility between connected pins
- required connections
- consistency of each node’s expected inputs and outputs

### Why it matters

Without this step, the generator could produce incoherent code or try to emit invalid dependencies.

## 2. Topological Sort

Once the graph is valid, the nodes are ordered with a Kahn-style topological sort.

### Purpose of the sort

- ensure a node is generated after its data dependencies
- keep the emission order stable
- make the final C++ easier to emit sequentially

### Important note

The topological order is the base, but the generator can still defer some nodes when they belong to the body of a control block. This avoids emitting loop or branch calculations too early.

## 3. Code Emission

`CodegenContext` then transforms nodes into C++ lines.

### Node types supported today

- `Literal`: emits a C++ constant (`lit_0`, `lit_1`, etc.)
- `Variable`: emits a variable name that can be reused in expressions
- `Operator`: emits an arithmetic, logical, or comparison expression
- `Print`: emits a `std::cout`
- `Condition`: emits an `if` / `else` block
- `Loop`: emits a `while` block
- `For`: emits a `for` block

### Emission principle

The generator walks the ordered nodes and assigns a C++ symbol to the nodes that need to be materialized in the final file.

Examples of symbols:

- `lit_0` for a constant
- `tmp_3` for an intermediate operator
- `i` for a loop variable or a named variable

### Control-expression emission

The conditions for `if`, `while`, and `for` are handled specially:

- expressions needed by the control block are inlined when necessary
- body nodes are deferred until the block is emitted
- this avoids generating code that references a loop variable before it is declared

## 4. Control Blocks

Control nodes have dedicated handling.

### `Condition`

- reads the boolean condition from its input
- emits `if (...) {`
- emits the true branch nodes
- optionally emits `else { ... }`

### `Loop`

- reads the boolean condition from its input
- emits `while (...) {`
- emits the loop body nodes

### `For`

- reads the initialization, condition, and increment inputs
- emits `for (init; condition; increment) {`
- emits the loop body nodes

### Indentation

Statements inside blocks are emitted with an extra indentation level to keep the result readable.

## 5. Constant Folding

The generator can simplify some expressions when all inputs are known at generation time.

### Example

If the graph contains `3 + 4`, the generator can emit `7` directly.

### Configurable mode

Constant folding is optional:

- enabled: shorter, more optimized code
- disabled: output that more closely mirrors the original graph

## 6. File Export

`GraphExporter` takes the generated code and writes it to `examples/output/`.

### Export steps

- validate the export request
- generate the C++ content
- write the file to disk

## Algorithms Used

- **Graph validation**: detects structural inconsistencies
- **Kahn topological sort**: orders nodes
- **Targeted recursive emission**: generates subtrees and branches
- **Constant folding**: computes constant expressions ahead of time

## Current Limits

The system works well for the supported cases, but it still has limitations.

### Supported types

- `int`
- `float` / `double`
- `bool`
- `string`
- `void` for control flows

### Types not yet supported

- `char`
- `long`, `unsigned`, `size_t`
- arrays and containers (`std::vector`, etc.)
- structs and domain objects
- pointers and references
- user-defined generic or template types

### Functional limitations

- no `break` / `continue`
- no user-defined functions or function calls
- no `std::cin` / interactive input
- no native arrays or collections
- no intermediate IR pipeline
- no advanced global optimizations

## Future Improvements

Possible next additions:

1. support for more C++ types
2. real mutable variables and assignments
3. dedicated nodes for functions and function calls
4. `break` and `continue`
5. `std::cin` and richer I/O
6. arrays, vectors, and data structures
7. intermediate transformations before C++ emission
8. more advanced optimizations such as global dead-code elimination
9. better scope and local-variable handling

## Summary

The generator follows a classic chain:

**validation → topological sort → C++ emission → export**

The core algorithm is already solid for simple graphs, operators, printing, conditions, and loops. The next natural steps are broader type support, more control-flow features, and stronger optimizations.
