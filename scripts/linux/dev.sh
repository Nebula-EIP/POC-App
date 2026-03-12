#!/usr/bin/env bash
set -e

# ============================================================================
# Colors & Constants
# ============================================================================
if [ -t 1 ]; then
    RED=$'\033[0;31m'
    GREEN=$'\033[0;32m'
    YELLOW=$'\033[1;33m'
    BLUE=$'\033[0;34m'
    NC=$'\033[0m'
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

BUILD_DIR=build

# ============================================================================
# Defaults
# ============================================================================
BUILD_TYPE="Debug"
FORMAT_MODE="Check"
RUN_TESTS="False"
PARALLEL=$(nproc 2>/dev/null || echo 4)
INTERACTIVE_MODE=false

# ============================================================================
# Help message
# ============================================================================
show_help() {
    cat << EOF
${BLUE}Development Build & Test for Nebula${NC}

${GREEN}Usage:${NC}
  ./scripts/linux/dev.sh [OPTIONS]

${GREEN}OPTIONS:${NC}
  -h, --help                Show this help message
  -b, --build-type TYPE     Debug / Release (default: ${BUILD_TYPE})
  -f, --format-mode MODE    Check / Fix (default: ${FORMAT_MODE})
  -t, --run-tests BOOL      True / False (default: ${RUN_TESTS})
  -j, --parallel N          Number of parallel build jobs (default: ${PARALLEL})

${GREEN}EXAMPLES:${NC}
  # Interactive mode (ask questions):
  ./scripts/linux/dev.sh

  # Batch mode (use flags, skip questions):
  ./scripts/linux/dev.sh --build-type Release --format-mode Fix --run-tests True --parallel 8

  # Batch mode with some flags (others use defaults):
  ./scripts/linux/dev.sh --run-tests True --parallel 1

${GREEN}NOTES:${NC}
  - If no arguments provided, interactive mode is used (questions asked)
  - If any argument is provided, non-interactive mode is used (questions skipped)
EOF
}

# ============================================================================
# Parse arguments
# ============================================================================
if [ $# -eq 0 ]; then
    INTERACTIVE_MODE=true
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -b|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -f|--format-mode)
            FORMAT_MODE="$2"
            shift 2
            ;;
        -t|--run-tests)
            RUN_TESTS="$2"
            shift 2
            ;;
        -j|--parallel)
            PARALLEL="$2"
            shift 2
            ;;
        *)
            echo -e "${RED}Error: Unknown option: $1${NC}" >&2
            show_help
            exit 1
            ;;
    esac
done

# ============================================================================
# Validate inputs
# ============================================================================
BUILD_TYPE=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
BUILD_TYPE=${BUILD_TYPE^}
FORMAT_MODE=$(echo "$FORMAT_MODE" | tr '[:upper:]' '[:lower:]')
FORMAT_MODE=${FORMAT_MODE^}
RUN_TESTS=$(echo "$RUN_TESTS" | tr '[:upper:]' '[:lower:]')
RUN_TESTS=${RUN_TESTS^}

case "$BUILD_TYPE" in
    Debug|Release) ;;
    *)
        echo -e "${RED}Error: Invalid build type: $BUILD_TYPE (must be Debug or Release)${NC}" >&2
        exit 1
        ;;
esac

case "$FORMAT_MODE" in
    Check|Fix) ;;
    *)
        echo -e "${RED}Error: Invalid format mode: $FORMAT_MODE (must be Check or Fix)${NC}" >&2
        exit 1
        ;;
esac

case "$RUN_TESTS" in
    True|False) ;;
    *)
        echo -e "${RED}Error: Invalid run tests value: $RUN_TESTS (must be True or False)${NC}" >&2
        exit 1
        ;;
esac

if ! [[ "$PARALLEL" =~ ^[0-9]+$ ]] || [ "$PARALLEL" -lt 1 ]; then
    echo -e "${RED}Error: Invalid parallel jobs: $PARALLEL (must be positive integer)${NC}" >&2
    exit 1
fi

# ============================================================================
# Interactive mode
# ============================================================================
if [ "$INTERACTIVE_MODE" = true ]; then
    echo -e "${BLUE}=== Development Build & Test Nebula ===${NC}"

    echo -e "${YELLOW}Select build type:${NC}"
    echo "1) Debug (default)"
    echo "2) Release"
    read -p "Enter choice (1 or 2): " build_choice
    build_choice=${build_choice:-1}

    if [ "$build_choice" = "1" ]; then
        BUILD_TYPE="Debug"
    elif [ "$build_choice" = "2" ]; then
        BUILD_TYPE="Release"
    else
        echo -e "${RED}Invalid choice${NC}"
        exit 1
    fi

    echo -e "${YELLOW}Select format mode:${NC}"
    echo "1) Check (default)"
    echo "2) Fix"
    read -p "Enter choice (1 or 2): " format_choice
    format_choice=${format_choice:-1}

    if [ "$format_choice" = "1" ]; then
        FORMAT_MODE="Check"
    elif [ "$format_choice" = "2" ]; then
        FORMAT_MODE="Fix"
    else
        echo -e "${RED}Invalid choice${NC}"
        exit 1
    fi

    echo -e "${YELLOW}Select run tests:${NC}"
    echo "1) False (default)"
    echo "2) True"
    read -p "Enter choice (1 or 2): " test_choice
    test_choice=${test_choice:-1}

    if [ "$test_choice" = "1" ]; then
        RUN_TESTS="False"
    elif [ "$test_choice" = "2" ]; then
        RUN_TESTS="True"
    else
        echo -e "${RED}Invalid choice${NC}"
        exit 1
    fi

    echo -e "${YELLOW}Select parallel jobs:${NC}"
    read -p "Enter number of parallel jobs (default: ${PARALLEL}): " parallel_choice
    if [ -n "$parallel_choice" ]; then
        if [[ "$parallel_choice" =~ ^[0-9]+$ ]] && [ "$parallel_choice" -ge 1 ]; then
            PARALLEL="$parallel_choice"
        else
            echo -e "${RED}Invalid choice (must be positive integer)${NC}"
            exit 1
        fi
    fi
fi

# ============================================================================
# Execute workflow
# ============================================================================
echo -e "${BLUE}=== Development Build & Test Nebula ===${NC}"
echo -e "${GREEN}Config: BuildType=${BUILD_TYPE}, Format=${FORMAT_MODE}, Tests=${RUN_TESTS}, Parallel=${PARALLEL}${NC}\n"

echo -e "${GREEN}[1/5] Configure...${NC}"
./scripts/linux/configure.sh ${BUILD_TYPE}

echo -e "${GREEN}[2/5] Format (${FORMAT_MODE})...${NC}"
./scripts/linux/format.sh ${FORMAT_MODE}

echo -e "${GREEN}[3/5] Lint...${NC}"
./scripts/linux/lint.sh ${PARALLEL}

echo -e "${GREEN}[4/5] Build (${BUILD_TYPE})...${NC}"
./scripts/linux/build.sh ${BUILD_TYPE} ${PARALLEL}

if [ "$RUN_TESTS" = "True" ]; then
    echo -e "${GREEN}[5/5] Test (${BUILD_TYPE})...${NC}"
    ./scripts/linux/test.sh ${BUILD_TYPE} ${PARALLEL}
else
    echo -e "${YELLOW}[5/5] Skipping tests...${NC}"
fi

echo -e "${BLUE}=== Development Completed ===${NC}"
