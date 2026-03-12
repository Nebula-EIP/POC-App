#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Debug}
PARALLEL=${2:-4}

echo "Building Nebula (${BUILD_TYPE}) with ${PARALLEL} parallel jobs..."

cmake --build ${BUILD_DIR} \
      --parallel ${PARALLEL}

if [ -f "${BUILD_DIR}/bin/${BUILD_TYPE}/Nebula" ]; then
    cp "${BUILD_DIR}/bin/${BUILD_TYPE}/Nebula" ./Nebula
fi

echo "Build completed."
