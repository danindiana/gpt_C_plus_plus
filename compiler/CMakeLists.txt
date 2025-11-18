cmake_minimum_required(VERSION 3.25)
project(CompilerOptimizationTester
    VERSION 1.0.0
    DESCRIPTION "Advanced Compiler Optimization Inconsistency Detection Framework"
    LANGUAGES CXX
)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wsign-conversion
        -Wcast-qual
        -Wformat=2
        -Wundef
        -Wshadow
        -Wcast-align
        -Wunused
        -Wnull-dereference
        -Wdouble-promotion
        -Wimplicit-fallthrough
    )

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options(-g3 -O0 -fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    else()
        add_compile_options(-O3 -march=native)
    endif()
endif()

# Find required packages
find_package(Threads REQUIRED)

# Optional: Find Google Test for unit testing
find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    message(STATUS "Google Test found - unit tests enabled")
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Source files
set(SOURCES
    src/program_generator.cpp
    src/information_injector.cpp
    src/compiler_interface.cpp
    src/oracle.cpp
    src/reporter.cpp
)

# Header files (for IDE support)
set(HEADERS
    include/compiler_tester/program_generator.hpp
    include/compiler_tester/information_injector.hpp
    include/compiler_tester/compiler_interface.hpp
    include/compiler_tester/oracle.hpp
    include/compiler_tester/reporter.hpp
)

# Create library
add_library(compiler_tester_lib STATIC ${SOURCES} ${HEADERS})
target_link_libraries(compiler_tester_lib PUBLIC Threads::Threads)

# Main executable
add_executable(compiler_tester src/main.cpp)
target_link_libraries(compiler_tester PRIVATE compiler_tester_lib)

# Installation rules
install(TARGETS compiler_tester compiler_tester_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/compiler_tester
    DESTINATION include
)

# Testing
if(GTest_FOUND)
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Testing: ${GTest_FOUND}")
message(STATUS "")
