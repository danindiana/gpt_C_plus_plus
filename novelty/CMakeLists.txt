cmake_minimum_required(VERSION 3.15)
project(NoveltyDetection VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to use C++20
option(USE_CXX20 "Use C++20 standard" OFF)
if(USE_CXX20)
    set(CMAKE_CXX_STANDARD 20)
endif()

# Build options
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build tests" ON)
option(USE_OPENMP "Enable OpenMP for parallelization" OFF)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3 -march=native)
    endif()
endif()

# Address Sanitizer
if(ENABLE_ASAN)
    if(NOT MSVC)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

# OpenMP support
if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        add_compile_definitions(USE_OPENMP)
    endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Header-only library target
add_library(novelty_detection INTERFACE)
target_include_directories(novelty_detection INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(novelty_detection INTERFACE cxx_std_17)

if(USE_OPENMP AND OpenMP_CXX_FOUND)
    target_link_libraries(novelty_detection INTERFACE OpenMP::OpenMP_CXX)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS novelty_detection
    EXPORT NoveltyDetectionTargets
    INCLUDES DESTINATION include
)

install(EXPORT NoveltyDetectionTargets
    FILE NoveltyDetectionTargets.cmake
    NAMESPACE NoveltyDetection::
    DESTINATION lib/cmake/NoveltyDetection
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/NoveltyDetectionConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/NoveltyDetectionConfig.cmake"
    INSTALL_DESTINATION lib/cmake/NoveltyDetection
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/NoveltyDetectionConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/NoveltyDetectionConfigVersion.cmake"
    DESTINATION lib/cmake/NoveltyDetection
)

# Print configuration summary
message(STATUS "")
message(STATUS "==================== Novelty Detection Configuration ====================")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:       C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:           ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build examples:     ${BUILD_EXAMPLES}")
message(STATUS "Build tests:        ${BUILD_TESTS}")
message(STATUS "OpenMP:             ${USE_OPENMP}")
message(STATUS "Address Sanitizer:  ${ENABLE_ASAN}")
message(STATUS "Warnings:           ${ENABLE_WARNINGS}")
message(STATUS "==========================================================================")
message(STATUS "")
