#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Debug}

echo "Building Nebula (${BUILD_TYPE})..."

cmake -B ${BUILD_DIR} \
      -S . \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCPM_SOURCE_CACHE=${PWD}/.cache/CPM

cmake --build ${BUILD_DIR} \
      --parallel

if [ -f "${BUILD_DIR}/bin/${BUILD_TYPE}/Nebula" ]; then
    cp "${BUILD_DIR}/bin/${BUILD_TYPE}/Nebula" ./Nebula
fi

echo "Build completed."
