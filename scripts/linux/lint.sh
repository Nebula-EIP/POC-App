#!/usr/bin/env bash
set -e

BUILD_DIR=build

echo "Running clang-tidy..."

TIDY_EXTRA_ARGS=(--extra-arg=-std=c++23)

if [ -f "/usr/include/c++/v1/expected" ]; then
    echo "libc++ detected: enabling -stdlib=libc++ for lint"
    TIDY_EXTRA_ARGS+=(--extra-arg=-stdlib=libc++)
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
        "${TIDY_EXTRA_ARGS[@]}"; then
        FAILED=1
    fi
done

if [ $FAILED -ne 0 ]; then
    echo "Lint errors detected"
    exit 1
fi

echo "Lint passed."
