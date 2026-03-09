#!/usr/bin/env bash
set -e

MODE=${1:-fix}

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

if [ "${MODE}" = "check" ]; then
    echo "Running clang-format check..."
    clang-format --dry-run --Werror $FILES
    echo "Format check passed."
else
    echo "Running clang-format..."
    clang-format -i $FILES
    echo "Formatting applied."
fi
