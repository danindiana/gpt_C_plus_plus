cmake_minimum_required(VERSION 3.20)

project(REALM
    VERSION 1.0.0
    DESCRIPTION "Retrieval-Augmented Language Model (REALM) Implementation in Modern C++"
    LANGUAGES CXX
)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(REALM_BUILD_TESTS "Build tests" ON)
option(REALM_BUILD_EXAMPLES "Build examples" ON)
option(REALM_BUILD_DOCS "Build documentation" OFF)
option(REALM_USE_FAISS "Use FAISS for efficient retrieval" OFF)
option(REALM_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wformat=2
    )

    if(REALM_ENABLE_SANITIZERS)
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
elseif(MSVC)
    add_compile_options(
        /W4
        /WX
        /permissive-
    )
endif()

# Optimization flags for Release build
if(CMAKE_BUILD_TYPE MATCHES "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-O3 -march=native)
    elseif(MSVC)
        add_compile_options(/O2)
    endif()
endif()

# Find dependencies
find_package(Threads REQUIRED)

# Optional: FAISS for high-performance retrieval
if(REALM_USE_FAISS)
    find_package(faiss QUIET)
    if(faiss_FOUND)
        add_compile_definitions(REALM_HAS_FAISS)
    else()
        message(WARNING "FAISS not found, using brute-force retrieval")
    endif()
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Source files
set(REALM_SOURCES
    src/types.cpp
    src/document_encoder.cpp
    src/mips.cpp
    src/retriever.cpp
    src/generator.cpp
    src/rag_model.cpp
)

# Create library
add_library(realm STATIC ${REALM_SOURCES})

target_include_directories(realm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(realm PUBLIC
    Threads::Threads
)

if(REALM_USE_FAISS AND faiss_FOUND)
    target_link_libraries(realm PUBLIC faiss)
endif()

# Set target properties
set_target_properties(realm PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/realm/types.hpp;include/realm/document_encoder.hpp;include/realm/mips.hpp;include/realm/retriever.hpp;include/realm/generator.hpp;include/realm/rag_model.hpp"
)

# Build examples
if(REALM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build tests
if(REALM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS realm
    EXPORT realmTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/realm
)

install(EXPORT realmTargets
    FILE realmTargets.cmake
    NAMESPACE realm::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/realm
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/realmConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/realmConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/realm
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/realmConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/realmConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/realmConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/realm
)

# Print configuration
message(STATUS "")
message(STATUS "REALM Configuration:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:      C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests:       ${REALM_BUILD_TESTS}")
message(STATUS "  Build Examples:    ${REALM_BUILD_EXAMPLES}")
message(STATUS "  Use FAISS:         ${REALM_USE_FAISS}")
message(STATUS "  Enable Sanitizers: ${REALM_ENABLE_SANITIZERS}")
message(STATUS "")
