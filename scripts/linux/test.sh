#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Release}

echo "Running tests (${BUILD_TYPE})..."

if [ ! -d "${BUILD_DIR}" ]; then
    echo "Build directory not found"
    exit 1
fi

ctest --test-dir ${BUILD_DIR} \
      -C ${BUILD_TYPE} \
      --output-on-failure \
      --verbose

echo "All tests passed."
