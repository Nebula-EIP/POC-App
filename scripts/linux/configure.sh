#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Debug}

echo "Configuring Nebula (${BUILD_TYPE})..."

cmake -B ${BUILD_DIR} \
      -S . \
      -G Ninja \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCPM_SOURCE_CACHE=${PWD}/.cache/CPM

echo "Configuration completed."
