#!/usr/bin/env bash
set -e

BUILD_DIR=build
BUILD_TYPE=${1:-Debug}
PARALLEL=${2:-4}

echo "Building Nebula (${BUILD_TYPE}) with ${PARALLEL} parallel jobs..."

cmake --build ${BUILD_DIR} \
      --parallel ${PARALLEL}

MODULE_DIR="${BUILD_DIR}/manual-modules"
mkdir -p "${MODULE_DIR}"
MODULE_FILE="${BUILD_DIR}/bin/${BUILD_TYPE}/fixture_valid_module.so"
if [ ! -f "${MODULE_FILE}" ]; then
    echo "Valid module fixture was not built" >&2
    exit 1
fi
cp "${MODULE_FILE}" "${MODULE_DIR}/"

if [ ! -f config.json ]; then
    printf '{\n  "module_folder": "%s"\n}\n' "${MODULE_DIR}" > config.json
    echo "Created default config.json"
fi

if [ -f "${BUILD_DIR}/bin/${BUILD_TYPE}/Nebula" ]; then
    cp "${BUILD_DIR}/bin/${BUILD_TYPE}/Nebula" ./Nebula
fi

echo "Build completed."
