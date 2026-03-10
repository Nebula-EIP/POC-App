#!/usr/bin/env bash
set -e

BUILD_DIR=build

echo "Running clang-tidy..."

FILES=$(find src -type f -name "*.cpp")

if [ -z "$FILES" ]; then
    echo "No source files found"
    exit 0
fi

FAILED=0

for file in $FILES; do
    echo "Checking $file"
    if ! clang-tidy "$file" \
        -p "${BUILD_DIR}" \
        --quiet \
        --extra-arg=-std=c++23; then
        FAILED=1
    fi
done

if [ $FAILED -ne 0 ]; then
    echo "Lint errors detected"
    exit 1
fi

echo "Lint passed."
