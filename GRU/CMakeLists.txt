cmake_minimum_required(VERSION 3.20)

project(GRU
    VERSION 1.0.0
    DESCRIPTION "Modern C++20 implementation of Gated Recurrent Unit (GRU)"
    LANGUAGES CXX
)

# C++20 standard required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(GRU_BUILD_EXAMPLES "Build example programs" ON)
option(GRU_BUILD_TESTS "Build unit tests" ON)
option(GRU_ENABLE_WARNINGS "Enable compiler warnings" ON)

# Compiler warnings
if(GRU_ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX)
    else()
        add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wconversion
            -Wsign-conversion
            -Wshadow
        )
    endif()
endif()

# GRU library
add_library(gru_lib
    src/gru.cpp
)

target_include_directories(gru_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(gru_lib PUBLIC cxx_std_20)

# Example programs
if(GRU_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(GRU_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS gru_lib
    EXPORT GRUTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT GRUTargets
    FILE GRUTargets.cmake
    NAMESPACE GRU::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GRU
)

# Package config
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GRUConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/GRUConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GRU
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/GRUConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/GRUConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/GRUConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GRU
)
