cmake_minimum_required(VERSION 3.20)

project(MLC
    VERSION 1.0.0
    DESCRIPTION "Meta-Learning for Compositionality - C++20 Implementation"
    LANGUAGES CXX
)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(MLC_BUILD_TESTS "Build tests" ON)
option(MLC_BUILD_EXAMPLES "Build examples" ON)
option(MLC_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(MLC_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(MLC_ENABLE_LTO "Enable link-time optimization" OFF)
option(MLC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Compiler warnings
add_library(mlc_warnings INTERFACE)
target_compile_options(mlc_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wformat=2
        -Wimplicit-fallthrough
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /permissive-
        /w14640
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /we4289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14905
        /w14906
        /w14928
    >
)

if(MLC_WARNINGS_AS_ERRORS)
    target_compile_options(mlc_warnings INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
    )
endif()

# Sanitizers
if(MLC_ENABLE_SANITIZERS AND NOT MSVC)
    add_library(mlc_sanitizers INTERFACE)
    target_compile_options(mlc_sanitizers INTERFACE
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )
    target_link_options(mlc_sanitizers INTERFACE
        -fsanitize=address
        -fsanitize=undefined
    )
endif()

# Link-time optimization
if(MLC_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(WARNING "LTO not supported: ${ipo_output}")
    endif()
endif()

# MLC library (header-only)
add_library(mlc INTERFACE)
add_library(MLC::mlc ALIAS mlc)

target_include_directories(mlc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(mlc INTERFACE cxx_std_20)

target_link_libraries(mlc INTERFACE
    mlc_warnings
)

if(TARGET mlc_sanitizers)
    target_link_libraries(mlc INTERFACE mlc_sanitizers)
endif()

# Main executable
add_executable(mlc_demo src/main.cpp)
target_link_libraries(mlc_demo PRIVATE MLC::mlc)

# Examples
if(MLC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(MLC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Benchmarks
if(MLC_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS mlc mlc_warnings
    EXPORT MLCTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/mlc
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT MLCTargets
    FILE MLCTargets.cmake
    NAMESPACE MLC::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MLC
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MLCConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MLCConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MLC
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MLCConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MLCConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MLCConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MLC
)

# Package generation
set(CPACK_PACKAGE_NAME "MLC")
set(CPACK_PACKAGE_VENDOR "MLC Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Meta-Learning for Compositionality Framework")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

# Print build configuration
message(STATUS "")
message(STATUS "═══════════════════════════════════════════════════")
message(STATUS "MLC Build Configuration")
message(STATUS "═══════════════════════════════════════════════════")
message(STATUS "Version:              ${PROJECT_VERSION}")
message(STATUS "C++ Standard:         C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Tests:          ${MLC_BUILD_TESTS}")
message(STATUS "Build Examples:       ${MLC_BUILD_EXAMPLES}")
message(STATUS "Build Benchmarks:     ${MLC_BUILD_BENCHMARKS}")
message(STATUS "Enable Sanitizers:    ${MLC_ENABLE_SANITIZERS}")
message(STATUS "Enable LTO:           ${MLC_ENABLE_LTO}")
message(STATUS "Warnings as Errors:   ${MLC_WARNINGS_AS_ERRORS}")
message(STATUS "Install Prefix:       ${CMAKE_INSTALL_PREFIX}")
message(STATUS "═══════════════════════════════════════════════════")
message(STATUS "")
