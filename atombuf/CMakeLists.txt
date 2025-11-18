cmake_minimum_required(VERSION 3.12)
project(AtomicBuffer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wnull-dereference
        -Wuseless-cast
        -Wdouble-promotion
        -Wformat=2
    )
elseif(MSVC)
    add_compile_options(
        /W4
        /permissive-
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
    )
endif()

# Find Threads package (required for std::thread)
find_package(Threads REQUIRED)

# Enable testing
enable_testing()
include(CTest)

# Header-only library interface
add_library(AtomicBuffer INTERFACE)
target_include_directories(AtomicBuffer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(AtomicBuffer INTERFACE cxx_std_17)
target_link_libraries(AtomicBuffer INTERFACE Threads::Threads)

# Basic test executable
add_executable(atomic_buffer_test AtomicBufferTest.cpp)
target_link_libraries(atomic_buffer_test PRIVATE AtomicBuffer)
add_test(NAME BasicTest COMMAND atomic_buffer_test)

# Stress test executable
add_executable(atomic_buffer_stress_test AtomicBufferStressTest.cpp)
target_link_libraries(atomic_buffer_stress_test PRIVATE AtomicBuffer)
add_test(NAME StressTest COMMAND atomic_buffer_stress_test)

# Detailed test executable
add_executable(atomic_buffer_detailed_test AtomicBufferDetailedTest.cpp)
target_link_libraries(atomic_buffer_detailed_test PRIVATE AtomicBuffer)
add_test(NAME DetailedTest COMMAND atomic_buffer_detailed_test)

# Logged test executable
add_executable(atomic_buffer_logged_test AtomicBufferLoggedTest.cpp)
target_link_libraries(atomic_buffer_logged_test PRIVATE AtomicBuffer)
add_test(NAME LoggedTest COMMAND atomic_buffer_logged_test)

# Terminal test executable
add_executable(atomic_buffer_terminal_test AtomicBufferTerminalTest.cpp)
target_link_libraries(atomic_buffer_terminal_test PRIVATE AtomicBuffer)
add_test(NAME TerminalTest COMMAND atomic_buffer_terminal_test)

# Fixed version test
add_executable(atomic_buffer_fixed_test
    AtomicBufferFixedTest.cpp
)
target_link_libraries(atomic_buffer_fixed_test PRIVATE AtomicBuffer)
add_test(NAME FixedTest COMMAND atomic_buffer_fixed_test)

# Benchmark executable (only if requested)
option(BUILD_BENCHMARKS "Build benchmark executables" ON)
if(BUILD_BENCHMARKS)
    add_executable(atomic_buffer_benchmark AtomicBufferBenchmark.cpp)
    target_link_libraries(atomic_buffer_benchmark PRIVATE AtomicBuffer)
endif()

# Sanitizer options
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(ENABLE_TSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
        add_link_options(-fsanitize=thread)
    endif()
endif()

if(ENABLE_UBSAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=undefined)
    endif()
endif()

# Installation rules
install(FILES
    AtomicBuffer.hpp
    AtomicBufferFixed.hpp
    AtomicBufferLogged.hpp
    DESTINATION include/atombuf
)

install(TARGETS AtomicBuffer
    EXPORT AtomicBufferTargets
    INCLUDES DESTINATION include
)

install(EXPORT AtomicBufferTargets
    FILE AtomicBufferTargets.cmake
    NAMESPACE AtomicBuffer::
    DESTINATION lib/cmake/AtomicBuffer
)

# Generate package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/AtomicBufferConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/AtomicBufferConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/AtomicBufferConfig.cmake"
    INSTALL_DESTINATION lib/cmake/AtomicBuffer
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AtomicBufferConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AtomicBufferConfigVersion.cmake"
    DESTINATION lib/cmake/AtomicBuffer
)

# Print configuration summary
message(STATUS "")
message(STATUS "AtomicBuffer Configuration Summary")
message(STATUS "==================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "AddressSanitizer: ${ENABLE_ASAN}")
message(STATUS "ThreadSanitizer: ${ENABLE_TSAN}")
message(STATUS "UBSanitizer: ${ENABLE_UBSAN}")
message(STATUS "")
