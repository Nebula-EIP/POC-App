#!/usr/bin/env bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BUILD_DIR=build

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
echo "1) Check (dry-run, default)"
echo "2) Fix (modify files)"
read -p "Enter choice (1 or 2): " format_choice
format_choice=${format_choice:-1}

if [ "$format_choice" = "1" ]; then
    FORMAT_MODE="check"
elif [ "$format_choice" = "2" ]; then
    FORMAT_MODE="fix"
else
    echo -e "${RED}Invalid choice${NC}"
    exit 1
fi

echo -e "${YELLOW}Select test mode:${NC}"
echo "1) Don't run tests (default)"
echo "2) Run tests"
read -p "Enter choice (1 or 2): " test_choice
test_choice=${test_choice:-1}

if [ "$test_choice" = "1" ]; then
    RUN_TESTS=false
elif [ "$test_choice" = "2" ]; then
    RUN_TESTS=true
else
    echo -e "${RED}Invalid choice${NC}"
    exit 1
fi

echo -e "${GREEN}Building ${BUILD_TYPE} with format mode: ${FORMAT_MODE}${NC} and test mode: ${RUN_TESTS}${NC}\n"

echo -e "${GREEN}[1/5] Configure...${NC}"
./scripts/linux/configure.sh ${BUILD_TYPE}

echo -e "${GREEN}[2/5] Format (${FORMAT_MODE})...${NC}"
./scripts/linux/format.sh ${FORMAT_MODE}

echo -e "${GREEN}[3/5] Lint...${NC}"
./scripts/linux/lint.sh

echo -e "${GREEN}[4/5] Build (${BUILD_TYPE})...${NC}"
./scripts/linux/build.sh ${BUILD_TYPE}

if [ "$RUN_TESTS" = true ]; then
    echo -e "${GREEN}[5/5] Test (${BUILD_TYPE})...${NC}"
    ./scripts/linux/test.sh ${BUILD_TYPE}
else
    echo -e "${YELLOW}[5/5] Skipping tests...${NC}"
fi

echo -e "${BLUE}=== Development Completed ===${NC}"
