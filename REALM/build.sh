#!/bin/bash
# REALM Build Script

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default options
BUILD_TYPE="Release"
BUILD_DIR="build"
NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
BUILD_TESTS=ON
BUILD_EXAMPLES=ON
RUN_TESTS=true
CLEAN=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --no-tests)
            BUILD_TESTS=OFF
            RUN_TESTS=false
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES=OFF
            shift
            ;;
        --jobs)
            NUM_JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug          Build in Debug mode (default: Release)"
            echo "  --release        Build in Release mode"
            echo "  --clean          Clean build directory before building"
            echo "  --no-tests       Don't build or run tests"
            echo "  --no-examples    Don't build examples"
            echo "  --jobs N         Use N parallel jobs (default: $NUM_JOBS)"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${GREEN}=== REALM Build Script ===${NC}"
echo ""
echo "Configuration:"
echo "  Build Type: $BUILD_TYPE"
echo "  Build Directory: $BUILD_DIR"
echo "  Parallel Jobs: $NUM_JOBS"
echo "  Build Tests: $BUILD_TESTS"
echo "  Build Examples: $BUILD_EXAMPLES"
echo "  Run Tests: $RUN_TESTS"
echo ""

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DREALM_BUILD_TESTS="$BUILD_TESTS" \
      -DREALM_BUILD_EXAMPLES="$BUILD_EXAMPLES" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . -j"$NUM_JOBS"

# Run tests if enabled
if [ "$RUN_TESTS" = true ] && [ "$BUILD_TESTS" = "ON" ]; then
    echo ""
    echo -e "${GREEN}Running tests...${NC}"
    ctest --output-on-failure -j"$NUM_JOBS"
fi

echo ""
echo -e "${GREEN}=== Build Complete ===${NC}"
echo ""
echo "Binaries located in: $BUILD_DIR/"
if [ "$BUILD_EXAMPLES" = "ON" ]; then
    echo ""
    echo "Run examples:"
    echo "  ./examples/simple_rag"
    echo "  ./examples/document_retrieval"
    echo "  ./examples/train_rag"
    echo "  ./examples/mips_benchmark"
fi

cd ..
