# Examples CMakeLists.txt

# Basic usage example
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE novelty_detection)

# Statistical detector example
add_executable(statistical_example statistical_example.cpp)
target_link_libraries(statistical_example PRIVATE novelty_detection)

# Memory detector example
add_executable(memory_example memory_example.cpp)
target_link_libraries(memory_example PRIVATE novelty_detection)

# Multi-strategy ensemble example
add_executable(ensemble_example ensemble_example.cpp)
target_link_libraries(ensemble_example PRIVATE novelty_detection)

# Streaming data example
add_executable(streaming_data streaming_data.cpp)
target_link_libraries(streaming_data PRIVATE novelty_detection)

# Installation
install(TARGETS
    basic_usage
    statistical_example
    memory_example
    ensemble_example
    streaming_data
    DESTINATION bin/examples
)
