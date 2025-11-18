cmake_minimum_required(VERSION 3.16)
project(PIPER_Kernels LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TT-Metal installation path (set via environment variable)
if(DEFINED ENV{TT_METAL_HOME})
    set(TT_METAL_HOME $ENV{TT_METAL_HOME})
else()
    set(TT_METAL_HOME "$ENV{HOME}/tt-metal")
endif()

message(STATUS "TT-Metal home: ${TT_METAL_HOME}")

# Find TT-Metal includes and libraries
find_path(TT_METAL_INCLUDE_DIR
    NAMES tt_metal/host_api.hpp
    PATHS ${TT_METAL_HOME}
    NO_DEFAULT_PATH
)

find_library(TT_METAL_LIB
    NAMES tt_metal
    PATHS ${TT_METAL_HOME}/build/lib
    NO_DEFAULT_PATH
)

if(NOT TT_METAL_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find TT-Metal headers. Set TT_METAL_HOME environment variable.")
endif()

if(NOT TT_METAL_LIB)
    message(FATAL_ERROR "Could not find TT-Metal library. Build TT-Metal first.")
endif()

# Include directories
include_directories(
    ${TT_METAL_INCLUDE_DIR}
    ${TT_METAL_HOME}/tt_metal
    ${TT_METAL_HOME}/tt_metal/common
)

# Neg2Zero kernel executable
add_executable(neg2zero_kernel neg2zero_kernel.cpp)

target_link_libraries(neg2zero_kernel
    PRIVATE
    ${TT_METAL_LIB}
    pthread
    dl
)

# Compiler flags
target_compile_options(neg2zero_kernel PRIVATE
    -Wall
    -Wextra
    -O3
)

# Install target
install(TARGETS neg2zero_kernel
    RUNTIME DESTINATION bin
)

# Print configuration
message(STATUS "==============================================")
message(STATUS "PIPER Kernels Configuration")
message(STATUS "==============================================")
message(STATUS "  TT-Metal home: ${TT_METAL_HOME}")
message(STATUS "  Include dir:   ${TT_METAL_INCLUDE_DIR}")
message(STATUS "  Library:       ${TT_METAL_LIB}")
message(STATUS "==============================================")
