#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Debug}

echo "Running tests (${BUILD_TYPE})..."

ctest --test-dir ${BUILD_DIR} \
      -C ${BUILD_TYPE} \
      --output-on-failure

echo "All tests passed."
