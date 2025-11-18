# Unit tests

# For now, create a simple test executable without external dependencies
# In the future, integrate Google Test or Catch2

add_executable(basic_tests
    basic_tests.cpp
)

target_link_libraries(basic_tests PRIVATE gru_lib)

# Add to CTest
add_test(NAME BasicTests COMMAND basic_tests)
