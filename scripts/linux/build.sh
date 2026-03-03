#!/bin/bash

# Build script for Linux

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Default values
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_AFTER_BUILD=false
BUILD_DIR="build"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        --run)
            RUN_AFTER_BUILD=true
            shift
            ;;
        -h|--help)
            echo "Usage: ./build.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -d, --debug     Build in Debug mode"
            echo "  -r, --release   Build in Release mode (default)"
            echo "  -c, --clean     Clean build directory before building"
            echo "  --run           Run the editor executable after building"
            echo "  -h, --help      Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${GREEN}=== POC-App Build Script ===${NC}"
echo -e "Build type: ${YELLOW}${BUILD_TYPE}${NC}"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf ${BUILD_DIR}
fi

# Create build directory
if [ ! -d "${BUILD_DIR}" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p ${BUILD_DIR}
fi

# Configure with CMake
echo -e "${YELLOW}Configuring project...${NC}"
cmake -B ${BUILD_DIR} -S . -DCMAKE_BUILD_TYPE=${BUILD_TYPE}

# Build
echo -e "${YELLOW}Building project...${NC}"
cmake --build ${BUILD_DIR} --config ${BUILD_TYPE} -j$(nproc)

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "Executable location: ${YELLOW}${BUILD_DIR}/bin/editor${NC}"

# Run if requested
if [ "$RUN_AFTER_BUILD" = true ]; then
    echo -e "${YELLOW}Running editor...${NC}"
    ./${BUILD_DIR}/bin/editor
fi
