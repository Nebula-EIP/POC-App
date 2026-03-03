#!/usr/bin/env bash
# Standalone code style checker - runs clang-format and clang-tidy on all src files

set -e

# Color codes for terminal output
RED="\033[1;31m"
GREEN="\033[1;32m"
YELLOW="\033[1;33m"
BLUE="\033[1;34m"
RESET="\033[0m"

echo -e "${BLUE}Code Style Checker${RESET}"
echo -e "${BLUE}(clang-format + clang-tidy)${RESET}"

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}clang-format is not installed or not in PATH${RESET}"
    exit 1
fi

# Check if clang-tidy is installed
if ! command -v clang-tidy &> /dev/null; then
    echo -e "${RED}clang-tidy is not installed or not in PATH${RESET}"
    exit 1
fi

# Check if build directory exists with compile_commands.json
if [ ! -f "build/compile_commands.json" ]; then
    echo -e "${YELLOW}compile_commands.json not found. Running CMake configure first...${RESET}"
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cd ..
fi

# Add folders to exclude here:
EXCLUDE_FOLDERS=(
    "*/utils/*"
)

# Build exclusion arguments for find
EXCLUDE_ARGS=""
for folder in "${EXCLUDE_FOLDERS[@]}"; do
    EXCLUDE_ARGS="$EXCLUDE_ARGS ! -path \"$folder\""
done

# ============ CLANG-FORMAT ============
echo ""
echo -e "${BLUE}Running clang-format on all source files...${RESET}"

FILES_TO_FORMAT=""
while IFS= read -r file; do
    FILES_TO_FORMAT="$FILES_TO_FORMAT $file"
done < <(eval "find src -type f \( -name \"*.cpp\" -o -name \"*.h\" -o -name \"*.tcc\" \) ! -path \"*/test/*\" ! -path \"*/.cache/*\" $EXCLUDE_ARGS")

if [ -z "$FILES_TO_FORMAT" ]; then
    echo -e "${YELLOW}No files to format.${RESET}"
else
    echo -e "${YELLOW}Formatting files...${RESET}"
    # Format all files in place
    echo $FILES_TO_FORMAT | xargs clang-format -i
    echo -e "${GREEN}✓ Formatting complete!${RESET}"
fi

# ============ CLANG-TIDY ============
echo ""
echo -e "${BLUE}Running clang-tidy on all source files...${RESET}"

TIDY_FAILED=0
while IFS= read -r file; do
    echo -e "${YELLOW}Checking: $file${RESET}"
    if ! clang-tidy -p build "$file" 2>&1; then
        echo -e "${RED}✗ Errors found in: $file${RESET}"
        TIDY_FAILED=1
    fi
done < <(eval "find src -type f \( -name \"*.cpp\" -o -name \"*.h\" -o -name \"*.tcc\" \) ! -path \"*/test/*\" ! -path \"*/.cache/*\" $EXCLUDE_ARGS")

if [ $TIDY_FAILED -eq 1 ]; then
    echo -e "${YELLOW}Fix the errors above, then run this script again.${RESET}"
    exit 1
fi

echo ""
echo -e "${GREEN}✓ All files passed style and tidy checks!${RESET}"
exit 0
