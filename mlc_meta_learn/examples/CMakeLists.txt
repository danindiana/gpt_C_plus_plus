# Examples for MLC Framework

# Simple example
add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE MLC::mlc)

# Compositional test
add_executable(compositional_test compositional_test.cpp)
target_link_libraries(compositional_test PRIVATE MLC::mlc)

# Add examples to build
set_target_properties(
    simple_example
    compositional_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
)

# Installation
install(TARGETS simple_example compositional_test
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)

message(STATUS "Configured MLC examples")
