#!/usr/bin/env bash

set -euo pipefail

# Resolve the project root.
# This script is located at:
#   <project-root>/scripts/linux/lint.sh
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/../.." && pwd)"

BUILD_DIR="${PROJECT_ROOT}/build"
SOURCE_DIR="${PROJECT_ROOT}/src"

JOBS="${1:-4}"

echo "Project root: ${PROJECT_ROOT}"
echo "Source directory: ${SOURCE_DIR}"
echo "Build directory: ${BUILD_DIR}"
echo "Running clang-tidy with ${JOBS} parallel jobs..."

# Validate the requested number of jobs.
if ! [[ "${JOBS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: JOBS must be a positive integer."
    exit 1
fi

# Check that the build directory exists.
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Error: build directory not found: ${BUILD_DIR}"
    echo "Run CMake configuration first."
    exit 1
fi

# Check that the source directory exists.
if [ ! -d "${SOURCE_DIR}" ]; then
    echo "Error: source directory not found: ${SOURCE_DIR}"
    exit 1
fi

# Use libc++ if its std::expected header is available.
if [ -f "/usr/include/c++/v1/expected" ]; then
    STDLIB_FLAG=( "--extra-arg=-stdlib=libc++" )
else
    STDLIB_FLAG=()
fi

# Collect only .cpp files from the project's root src/ directory.
mapfile -t FILES < <(
    find "${SOURCE_DIR}" \
        -type f \
        -name "*.cpp" \
        -print
)

if [ "${#FILES[@]}" -eq 0 ]; then
    echo "No source files found in ${SOURCE_DIR}"
    exit 0
fi

TMP_ERRORS="$(mktemp)"

trap 'rm -f "${TMP_ERRORS}"' EXIT

for file in "${FILES[@]}"; do
    (
        echo "Checking ${file}"

        if ! clang-tidy "${file}" \
            -p "${BUILD_DIR}" \
            --quiet \
            --header-filter="${SOURCE_DIR}/.*" \
            "${STDLIB_FLAG[@]}"; then

            echo "${file}" >> "${TMP_ERRORS}"
        fi
    ) &

    # Limit the number of concurrently running clang-tidy processes.
    while (( $(jobs -r -p | wc -l) >= JOBS )); do
        sleep 0.01
    done
done

wait

if [ -s "${TMP_ERRORS}" ]; then
    echo
    echo "Lint errors detected in:"
    cat "${TMP_ERRORS}"
    exit 1
fi

echo
echo "Lint passed."