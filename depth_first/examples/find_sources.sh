#!/bin/bash
# Example: Find all C++ source files in a project

PROJECT_DIR="${1:-.}"
MAX_DEPTH="${2:-10}"

echo "Finding C++ source files in: $PROJECT_DIR"
echo "Maximum depth: $MAX_DEPTH"
echo ""

# Find all .cpp files
echo "=== C++ Source Files (.cpp) ==="
../build/bin/dfs-search "*.cpp" "$PROJECT_DIR" "$MAX_DEPTH"

echo ""
echo "=== C++ Header Files (.hpp) ==="
../build/bin/dfs-search "*.hpp" "$PROJECT_DIR" "$MAX_DEPTH"

echo ""
echo "=== C Header Files (.h) ==="
../build/bin/dfs-search "*.h" "$PROJECT_DIR" "$MAX_DEPTH"
