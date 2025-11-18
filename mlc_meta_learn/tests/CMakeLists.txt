# Tests for MLC Framework

# Try to find Google Test
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    message(STATUS "Google Test not found, downloading...")

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
endif()

# Test executable
add_executable(mlc_tests
    test_types.cpp
    test_example.cpp
    test_episode.cpp
    test_tensor.cpp
    test_network.cpp
    test_utils.cpp
)

target_link_libraries(mlc_tests PRIVATE
    MLC::mlc
    GTest::gtest
    GTest::gtest_main
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(mlc_tests)

# Set test output directory
set_target_properties(mlc_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
)

message(STATUS "Configured MLC tests")
