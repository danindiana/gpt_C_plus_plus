cmake_minimum_required(VERSION 3.20)
project(DFSSearch
    VERSION 2.0.0
    DESCRIPTION "Modern C++20/23 Depth-First Search File Tools"
    LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -O3
        -march=native
    )
elseif(MSVC)
    add_compile_options(/W4 /O2)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Executable: dfs-search (simple version)
add_executable(dfs-search src/dfs_simple.cpp)
target_link_libraries(dfs-search stdc++ pthread)

# Installation rules
install(TARGETS dfs-search
    RUNTIME DESTINATION bin
    COMPONENT runtime)

# Install headers
install(DIRECTORY include/
    DESTINATION include/dfs_search
    FILES_MATCHING PATTERN "*.hpp")

# Package configuration
set(CPACK_PACKAGE_NAME "dfs-search")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_GENERATOR "TGZ;DEB")
include(CPack)

# Testing support
enable_testing()

# Test: Basic search in current directory
add_test(NAME test_basic_search
    COMMAND dfs-search "*.txt" ${CMAKE_CURRENT_SOURCE_DIR} 3)

# Test: Makefile search
add_test(NAME test_makefile_search
    COMMAND dfs-search "Makefile" ${CMAKE_CURRENT_SOURCE_DIR} 1)

# Build information
message(STATUS "")
message(STATUS "===========================================")
message(STATUS "  DFS Search Tools - Configuration")
message(STATUS "===========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "===========================================")
message(STATUS "")

# Custom targets
add_custom_target(run
    COMMAND dfs-search "*.cpp" ${CMAKE_CURRENT_SOURCE_DIR}/src 2
    DEPENDS dfs-search
    COMMENT "Running DFS search example...")

add_custom_target(info
    COMMAND ${CMAKE_COMMAND} -E echo "DFS Search Tools v${PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build directory: ${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Source directory: ${CMAKE_SOURCE_DIR}"
    COMMENT "Project information")
