#!/usr/bin/env bash
set -e

BUILD_TYPE=${1:-Release}
BUILD_DIR=build
LINT_BUILD_DIR="build-lint-${BUILD_TYPE}"

echo "Running clang-tidy..."

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy not found"
    exit 1
fi

if [ ! -f "${BUILD_DIR}/compile_commands.json" ]; then
    echo "compile_commands.json not found in ${BUILD_DIR}."
    echo "Generating a dedicated lint database in ${LINT_BUILD_DIR}..."

    cmake -B "${LINT_BUILD_DIR}" \
          -S . \
          -G Ninja \
          -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCPM_SOURCE_CACHE="${PWD}/.cache/CPM"

    BUILD_DIR="${LINT_BUILD_DIR}"
fi

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
        --extra-arg=-std=c++2b; then
        FAILED=1
    fi
done

if [ $FAILED -ne 0 ]; then
    echo "Lint errors detected"
    exit 1
fi

echo "Lint passed."
