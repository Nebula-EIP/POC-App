#!/usr/bin/env bash
set -e

BUILD_DIR=build
JOBS=${1:-4}

echo "Running clang-tidy with ${JOBS} parallel jobs..."

if [ -f "/usr/include/c++/v1/expected" ]; then
    STDLIB_FLAG="--extra-arg=-stdlib=libc++"
else
    STDLIB_FLAG=""
fi

FILES=$(find src -type f -name "*.cpp")

if [ -z "$FILES" ]; then
    echo "No source files found"
    exit 0
fi

FAILED=0
TMP_ERRORS=$(mktemp)
trap 'rm -f "$TMP_ERRORS"' EXIT

for file in $FILES; do
    (
        echo "Checking $file"
        if ! clang-tidy "$file" \
            -p "${BUILD_DIR}" \
            --quiet \
            ${STDLIB_FLAG}; then
            echo "$file" >> "$TMP_ERRORS"
        fi
    ) &

    while (( $(jobs -r -p | wc -l) >= JOBS )); do
        sleep 0.01
    done
done

wait

if [ -s "$TMP_ERRORS" ]; then
    echo "Lint errors detected in:"
    cat "$TMP_ERRORS"
    exit 1
fi

echo "Lint passed."
