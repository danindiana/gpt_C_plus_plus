# Tests CMakeLists.txt

# Note: This is a placeholder for future test implementation
# You can use Google Test, Catch2, or other testing frameworks

# Simple test executable (compile-time validation)
add_executable(compile_test compile_test.cpp)
target_link_libraries(compile_test PRIVATE novelty_detection)

# Add test
add_test(NAME CompileTest COMMAND compile_test)
