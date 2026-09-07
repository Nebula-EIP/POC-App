#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPOSITORY_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

BUILD_DIR="build-coverage"
BUILD_TYPE="Debug"

TEST_REGEX="${1:-Renderer(Component|Capability)Test}"
SOURCE_FILTER="${2:-src/new_core/modules/capabilities/renderer_capability.*}"
PARALLEL="${3:-$(nproc)}"

REPORT_PATH="${BUILD_DIR}/coverage/renderer-coverage.html"

show_help() {
    cat <<EOF

Renderer coverage report for Nebula

Usage:
  ./scripts/linux/coverage.sh [TEST_REGEX] [SOURCE_FILTER] [PARALLEL]

Defaults:
  TEST_REGEX    Renderer(Component|Capability)Test
  SOURCE_FILTER src/new_core/modules/capabilities/renderer_capability.*
  PARALLEL      number of available CPU cores

Examples:
  ./scripts/linux/coverage.sh
  ./scripts/linux/coverage.sh "RendererCapabilityTest" "src/new_core/.*" 4

EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    show_help
    exit 0
fi

# ---------------------------------------------------------------------------
# Validate arguments
# ---------------------------------------------------------------------------

if ! [[ "$PARALLEL" =~ ^[1-9][0-9]*$ ]]; then
    echo "Invalid parallel jobs: $PARALLEL (must be a positive integer)"
    exit 1
fi

# ---------------------------------------------------------------------------
# Check dependencies
# ---------------------------------------------------------------------------

if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake was not found in PATH."
    exit 1
fi

if ! command -v gcov >/dev/null 2>&1; then
    echo "gcov was not found in PATH."
    echo "Install GCC with:"
    echo "  sudo dnf install gcc-c++"
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "Python 3 was not found in PATH."
    exit 1
fi

if ! python3 -m gcovr --version >/dev/null 2>&1; then
    echo "gcovr is not installed."
    echo "Install it with:"
    echo "  python3 -m pip install --user gcovr"
    exit 1
fi

# ---------------------------------------------------------------------------
# Move to repository root
# ---------------------------------------------------------------------------

cd "$REPOSITORY_ROOT"

echo "========================================"
echo " Nebula Coverage"
echo "========================================"
echo "Build directory : $BUILD_DIR"
echo "Build type      : $BUILD_TYPE"
echo "Test regex      : $TEST_REGEX"
echo "Source filter   : $SOURCE_FILTER"
echo "Parallel jobs   : $PARALLEL"
echo "========================================"
echo

# ---------------------------------------------------------------------------
# [1/5] Configure
# ---------------------------------------------------------------------------

echo "[1/5] Configuring the instrumented build..."

cmake -S . -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_FLAGS="--coverage -O0 -g" \
    -DCMAKE_C_FLAGS="--coverage -O0 -g" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
    -DCPM_SOURCE_CACHE="$REPOSITORY_ROOT/.cache/CPM"

# ---------------------------------------------------------------------------
# [2/5] Build
# ---------------------------------------------------------------------------

echo "[2/5] Building unit_tests with coverage enabled..."

cmake --build "$BUILD_DIR" \
    --target unit_tests \
    --parallel "$PARALLEL"

# ---------------------------------------------------------------------------
# [3/5] Remove old coverage counters
# ---------------------------------------------------------------------------

echo "[3/5] Removing counters from previous runs..."

find "$BUILD_DIR" -name "*.gcda" -type f -delete

# ---------------------------------------------------------------------------
# [4/5] Run tests
# ---------------------------------------------------------------------------

echo "[4/5] Running selected tests..."

ctest \
    --test-dir "$BUILD_DIR" \
    --parallel "$PARALLEL" \
    --output-on-failure \
    -R "$TEST_REGEX"

# ---------------------------------------------------------------------------
# [5/5] Generate report
# ---------------------------------------------------------------------------

echo "[5/5] Generating the HTML report..."

mkdir -p "$BUILD_DIR/coverage"

python3 -m gcovr \
    --root . \
    --filter "$SOURCE_FILTER" \
    --gcov-executable gcov \
    --exclude-throw-branches \
    --exclude-unreachable-branches \
    --print-summary \
    --html-details "$REPORT_PATH" \
    "$BUILD_DIR"

echo
echo "========================================"
echo " Coverage report generated successfully"
echo "========================================"
echo
echo "Report:"
echo "  $REPOSITORY_ROOT/$REPORT_PATH"
echo

# Open the report if a graphical environment is available.
if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$REPORT_PATH" >/dev/null 2>&1 &
fi