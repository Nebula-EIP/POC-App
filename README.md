# Nebula - POC-App

Nebula is a visual programming environment that generates C++ code from a node graph. This repository is the proof-of-concept application validating the core engine: graph representation, real-time code generation, and file export.

> **Status:** Proof of concept - core engine under active development.

## Overview

Instead of writing code directly, users build programs by connecting nodes in a visual graph. Nebula translates that graph into compilable C++ code and writes it to disk.

This POC validates three things:
- the internal node graph model (connections, types, exceptions)
- the pipeline from graph to generated C++ source files
- the editor as the single executable entry point

## Architecture

| Component | Role |
|---|---|
| `core` | Node graph model shared across all libraries |
| `graph-builder` | Keeps a live graph in sync with C++ source files in a folder (runs in its own thread) |
| `graph-exporter` | Serializes a node graph to compilation-ready C++ files on disk |
| `editor` | Visual editor - the only executable of the project |
| `utils` | Shared utilities: logger, ID manager, exception base, string splitter |
| `cmake-tool` | CMake helper utilities |

## Prerequisites

- CMake ≥ 3.20
- A C++17-compatible compiler (GCC, Clang, or MSVC)
- Ninja (recommended)

## Quick Start

```bash
git clone https://github.com/Nebula-EIP/POC-App.git
cd POC-App
```

**Linux**
```bash
./scripts/linux/dev.sh
```

**Windows**
```bat
call .\scripts\windows\dev.bat
```

These scripts configure, build, and run the tests in one step.

## Project Structure

```
src/
  core/           Node graph model (graph, nodes, connections, exceptions)
  graph-builder/  Live graph ↔ folder synchronisation
  graph-exporter/ Graph → C++ file export
  editor/         Visual editor executable
  utils/          Shared utilities
  cmake-tool/     CMake helpers
tests/            Unit and integration tests
scripts/
  linux/          Linux build/dev scripts
  windows/        Windows build/dev scripts
docs/             Docusaurus documentation site
.github/
  workflows/      CI/CD pipelines (build, test, docs)
```

## Documentation

- Full docs: [nebula-eip.github.io/POC-App](https://nebula-eip.github.io/POC-App/)
- API reference (Doxygen): available from the docs navbar
- Developer setup guide: `docs/docs/dev/`

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request - it covers branching strategy, commit conventions, and local checks.

## Security

Report vulnerabilities privately following the process in [SECURITY.md](SECURITY.md).

## Support

See [SUPPORT.md](SUPPORT.md) for how to get help.

## License

This project is licensed under the terms of the [LICENSE](LICENSE) file.
