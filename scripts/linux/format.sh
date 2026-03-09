#!/usr/bin/env bash
set -e

echo "Running clang-format check..."

if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found"
    exit 1
fi

FILES=$(find src -type f \( \
    -name "*.cpp" -o \
    -name "*.hpp" -o \
    -name "*.h" -o \
    -name "*.tcc" \
\))

if [ -z "$FILES" ]; then
    echo "No source files found"
    exit 0
fi

clang-format --dry-run --Werror $FILES

echo "Format check passed."
