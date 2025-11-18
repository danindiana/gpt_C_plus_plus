cmake_minimum_required(VERSION 3.20)

project(Fortuna
    VERSION 1.0.0
    DESCRIPTION "Modern C++20 Bayesian Neural Network Library"
    LANGUAGES CXX
)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(FORTUNA_BUILD_EXAMPLES "Build example programs" ON)
option(FORTUNA_BUILD_TESTS "Build tests" ON)
option(FORTUNA_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(FORTUNA_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler flags
if(FORTUNA_ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4)
        if(FORTUNA_WARNINGS_AS_ERRORS)
            add_compile_options(/WX)
        endif()
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
        if(FORTUNA_WARNINGS_AS_ERRORS)
            add_compile_options(-Werror)
        endif()
    endif()
endif()

# Library sources
set(FORTUNA_SOURCES
    src/core/layer.cpp
    src/core/model.cpp
    src/core/data_loader.cpp
    src/bayesian/prob_classifier.cpp
    src/utils/activations.cpp
    src/utils/initializers.cpp
)

# Create main library
add_library(fortuna ${FORTUNA_SOURCES})

# Set include directories
target_include_directories(fortuna
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link math library on Unix systems
if(UNIX)
    target_link_libraries(fortuna PUBLIC m)
endif()

# Set library properties
set_target_properties(fortuna PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER include/fortuna/fortuna.hpp
)

# Examples
if(FORTUNA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS fortuna
    EXPORT FortunaTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fortuna
)

# Install headers
install(DIRECTORY include/fortuna
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Export targets
install(EXPORT FortunaTargets
    FILE FortunaTargets.cmake
    NAMESPACE Fortuna::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Fortuna
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FortunaConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FortunaConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Fortuna
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/FortunaConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/FortunaConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FortunaConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Fortuna
)

# Print configuration summary
message(STATUS "Fortuna Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Examples: ${FORTUNA_BUILD_EXAMPLES}")
message(STATUS "  Build Tests: ${FORTUNA_BUILD_TESTS}")
message(STATUS "  Enable Warnings: ${FORTUNA_ENABLE_WARNINGS}")
