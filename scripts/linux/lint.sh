#!/usr/bin/env bash
set -e

BUILD_DIR=build

echo "Running clang-tidy..."

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy not found"
    exit 1
fi

if [ ! -f "${BUILD_DIR}/compile_commands.json" ]; then
    echo "compile_commands.json not found. Configure the project first."
    exit 1
fi

FILES=$(find src -type f -name "*.cpp")

if [ -z "$FILES" ]; then
    echo "No source files found"
    exit 0
fi

FAILED=0

for file in $FILES; do
    echo "Checking $file"
    if ! clang-tidy "$file" -p ${BUILD_DIR}; then
        FAILED=1
    fi
done

if [ $FAILED -ne 0 ]; then
    echo "Lint errors detected"
    exit 1
fi

echo "Lint passed."
