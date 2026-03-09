#!/usr/bin/env bash
set -e

BUILD_TYPE=${1:-Release}
LINT_BUILD_DIR="build-lint-${BUILD_TYPE}"
BUILD_DIR="${LINT_BUILD_DIR}"

echo "Running clang-tidy..."

CLANG_TIDY_BIN=""
for candidate in clang-tidy-20 clang-tidy-19 clang-tidy-18 clang-tidy-17 clang-tidy; do
    if command -v "${candidate}" >/dev/null 2>&1; then
        CLANG_TIDY_BIN="${candidate}"
        break
    fi
done

if [ -z "${CLANG_TIDY_BIN}" ]; then
    echo "clang-tidy not found"
    exit 1
fi

echo "Using ${CLANG_TIDY_BIN}: $(${CLANG_TIDY_BIN} --version | head -n 1)"

CLANG_SUFFIX="${CLANG_TIDY_BIN#clang-tidy}"
CMAKE_COMPILER_ARGS=()

if [ -n "${CLANG_SUFFIX}" ] && command -v "clang++${CLANG_SUFFIX}" >/dev/null 2>&1 && command -v "clang${CLANG_SUFFIX}" >/dev/null 2>&1; then
    CMAKE_COMPILER_ARGS=(-DCMAKE_CXX_COMPILER="clang++${CLANG_SUFFIX}" -DCMAKE_C_COMPILER="clang${CLANG_SUFFIX}")
    echo "Using Clang toolchain for lint DB: clang++${CLANG_SUFFIX}"
elif command -v clang++ >/dev/null 2>&1 && command -v clang >/dev/null 2>&1; then
    CMAKE_COMPILER_ARGS=(-DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang)
    echo "Using default Clang toolchain for lint DB: clang++"
else
    echo "No Clang compiler found; using project default compiler for lint DB."
fi

if [ ! -f "${BUILD_DIR}/compile_commands.json" ]; then
    echo "Generating a dedicated lint database in ${BUILD_DIR}..."

        cmake -B "${LINT_BUILD_DIR}" \
                    -S . \
                    -G Ninja \
                    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
                    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
                    -DCMAKE_CXX_STANDARD=23 \
                    -DCPM_SOURCE_CACHE="${PWD}/.cache/CPM" \
                    "${CMAKE_COMPILER_ARGS[@]}"

fi

FILES=$(find src -type f -name "*.cpp")

if [ -z "$FILES" ]; then
    echo "No source files found"
    exit 0
fi

FAILED=0

for file in $FILES; do
    echo "Checking $file"
    if ! "${CLANG_TIDY_BIN}" "$file" \
        -p "${BUILD_DIR}" \
        --quiet \
        --extra-arg=-std=c++2b; then
        FAILED=1
    fi
done

if [ $FAILED -ne 0 ]; then
    echo "Lint errors detected"
    exit 1
fi

echo "Lint passed."
