#!/usr/bin/env bash
set -e

BUILD_DIR="build"
DEMO_NAME="codegen_demo"
PARALLEL=${1:-$(nproc 2>/dev/null || echo 4)}

if ! [[ "$PARALLEL" =~ ^[0-9]+$ ]] || [ "$PARALLEL" -lt 1 ]; then
    echo "Invalid parallel jobs: $PARALLEL (must be a positive integer)" >&2
    exit 1
fi

echo "Building ${DEMO_NAME} with ${PARALLEL} parallel jobs..."
cmake --build "${BUILD_DIR}" --target "${DEMO_NAME}" --parallel "${PARALLEL}"

DEMO_PATH=$(find "${BUILD_DIR}/bin" -type f -name "${DEMO_NAME}" -perm -u+x | head -n 1)
if [ -z "${DEMO_PATH}" ]; then
    echo "Could not find built demo executable for ${DEMO_NAME}" >&2
    exit 1
fi

echo "Running ${DEMO_PATH}"
exec "${DEMO_PATH}"