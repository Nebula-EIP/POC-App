#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Debug}
PARALLEL=${2:-4}

echo "Running tests (${BUILD_TYPE}) with ${PARALLEL} parallel jobs..."

ctest --test-dir ${BUILD_DIR} \
      -C ${BUILD_TYPE} \
      --parallel ${PARALLEL} \
      --output-on-failure

echo "All tests passed."
