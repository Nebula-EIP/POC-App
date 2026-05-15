#!/usr/bin/env bash
set -e

MODE=${1:-Check}

FILES=$(find src -type f \( \
    -name "*.cpp" -o \
    -name "*.hpp" -o \
    -name "*.tcc" \
\))

if [ -z "$FILES" ]; then
    echo "No source files found"
    exit 0
fi

if [ "${MODE}" = "Fix" ]; then
    echo "Running clang-format..."
    clang-format -i $FILES
    echo "Formatting applied."
else
    echo "Running clang-format check..."
    clang-format --dry-run --Werror $FILES
    echo "Format check passed."
fi
