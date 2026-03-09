#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Release}

echo "Building Nebula (${BUILD_TYPE})..."

mkdir -p ${BUILD_DIR}

cmake -B ${BUILD_DIR} \
      -S . \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCPM_SOURCE_CACHE=${PWD}/.cache/CPM

cmake --build ${BUILD_DIR} \
      --config ${BUILD_TYPE} \
      --parallel

echo "Build completed."