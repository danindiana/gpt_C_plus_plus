# Modern Makefile for DFS Search Tools
# Supports C++23 with latest GCC/Clang

# Compiler Configuration
CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Wpedantic -O3 -march=native
DEBUGFLAGS := -g -O0 -DDEBUG
INCLUDES := -Iinclude
LDFLAGS := -lstdc++ -lpthread

# Directories
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj

# Compiler version detection
COMPILER_VERSION := $(shell $(CXX) --version | head -n1)

# Targets
SIMPLE_TARGET := $(BIN_DIR)/dfs-search
IDDFS_TARGET := $(BIN_DIR)/iddfs-search

# Source files
SIMPLE_SRC := $(SRC_DIR)/dfs_simple.cpp
IDDFS_SRC := $(SRC_DIR)/iddfs_main.cpp
DFS_SRC := $(SRC_DIR)/dfs_main.cpp

# Headers
HEADERS := $(wildcard $(INC_DIR)/*.hpp)

# Colors for output
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
NC := \033[0m # No Color

.PHONY: all clean debug help info install test examples

# Default target
all: info dirs $(SIMPLE_TARGET)
	@echo "$(GREEN)✓ Build complete!$(NC)"
	@echo "$(BLUE)Run: $(SIMPLE_TARGET) --help$(NC)"

# Create build directories
dirs:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)

# Main executable
$(SIMPLE_TARGET): $(SIMPLE_SRC) $(HEADERS)
	@echo "$(YELLOW)Compiling $@...$(NC)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS)
	@echo "$(GREEN)✓ Built: $@$(NC)"

# Debug build
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: clean dirs $(SIMPLE_TARGET)
	@echo "$(GREEN)✓ Debug build complete$(NC)"

# Install to /usr/local/bin (requires sudo)
install: all
	@echo "$(YELLOW)Installing to /usr/local/bin...$(NC)"
	@sudo cp $(SIMPLE_TARGET) /usr/local/bin/dfs-search
	@sudo chmod +x /usr/local/bin/dfs-search
	@echo "$(GREEN)✓ Installed successfully$(NC)"

# Uninstall
uninstall:
	@echo "$(YELLOW)Removing from /usr/local/bin...$(NC)"
	@sudo rm -f /usr/local/bin/dfs-search
	@echo "$(GREEN)✓ Uninstalled$(NC)"

# Clean build artifacts
clean:
	@echo "$(YELLOW)Cleaning build artifacts...$(NC)"
	@rm -rf $(BUILD_DIR)
	@echo "$(GREEN)✓ Clean complete$(NC)"

# Display build information
info:
	@echo "$(BLUE)===================================$(NC)"
	@echo "$(BLUE)  DFS Search Tools - Build Info$(NC)"
	@echo "$(BLUE)===================================$(NC)"
	@echo "Compiler: $(COMPILER_VERSION)"
	@echo "C++ Standard: C++23"
	@echo "Optimization: -O3 -march=native"
	@echo "$(BLUE)===================================$(NC)"
	@echo ""

# Run basic tests
test: all
	@echo "$(YELLOW)Running tests...$(NC)"
	@echo "Test 1: Search for Makefile in current directory"
	@$(SIMPLE_TARGET) "Makefile" . 1
	@echo ""
	@echo "Test 2: Search for *.cpp files"
	@$(SIMPLE_TARGET) "*.cpp" src 3
	@echo "$(GREEN)✓ Tests complete$(NC)"

# Show usage examples
examples: all
	@echo "$(BLUE)===================================$(NC)"
	@echo "$(BLUE)         Usage Examples$(NC)"
	@echo "$(BLUE)===================================$(NC)"
	@echo ""
	@echo "1. Find all .txt files:"
	@echo "   $(SIMPLE_TARGET) '*.txt' /home/user 5"
	@echo ""
	@echo "2. Find files with regex (verbose):"
	@echo "   $(SIMPLE_TARGET) 'test.*\\.cpp' . 10 -r -v"
	@echo ""
	@echo "3. Quick search in current directory:"
	@echo "   $(SIMPLE_TARGET) '*.md' . 2"
	@echo ""
	@echo "$(BLUE)===================================$(NC)"

# Help target
help:
	@echo "$(BLUE)===================================$(NC)"
	@echo "$(BLUE)      DFS Search - Makefile$(NC)"
	@echo "$(BLUE)===================================$(NC)"
	@echo ""
	@echo "Available targets:"
	@echo "  $(GREEN)make$(NC)          - Build release version"
	@echo "  $(GREEN)make debug$(NC)    - Build debug version"
	@echo "  $(GREEN)make clean$(NC)    - Remove build artifacts"
	@echo "  $(GREEN)make install$(NC)  - Install to system (requires sudo)"
	@echo "  $(GREEN)make uninstall$(NC)- Remove from system"
	@echo "  $(GREEN)make test$(NC)     - Run test suite"
	@echo "  $(GREEN)make examples$(NC) - Show usage examples"
	@echo "  $(GREEN)make info$(NC)     - Show build configuration"
	@echo "  $(GREEN)make help$(NC)     - Show this help"
	@echo ""
	@echo "$(BLUE)===================================$(NC)"

# Check compiler requirements
check-compiler:
	@echo "Checking compiler support for C++23..."
	@$(CXX) -std=c++23 -x c++ -E -v - < /dev/null 2>&1 | grep -q "c++23" || \
		(echo "$(RED)Error: Compiler does not support C++23$(NC)" && exit 1)
	@echo "$(GREEN)✓ Compiler supports C++23$(NC)"
