cmake_minimum_required(VERSION 3.20)

# Download and setup Google Test
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable Google Test
include(GoogleTest)

# Test files
set(TEST_SOURCES
    test_types.cpp
    test_document_encoder.cpp
    test_mips.cpp
    test_retriever.cpp
    test_generator.cpp
    test_rag_model.cpp
)

add_executable(realm_tests ${TEST_SOURCES})

target_link_libraries(realm_tests PRIVATE
    realm
    GTest::gtest_main
    GTest::gmock
)

# Discover tests
gtest_discover_tests(realm_tests)

# Add custom target to run tests
add_custom_target(check
    COMMAND realm_tests
    DEPENDS realm_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests..."
)
