# Examples CMakeLists.txt

# Simple classification example
add_executable(simple_classification simple_classification.cpp)
target_link_libraries(simple_classification PRIVATE fortuna)

# Bayesian regression example
add_executable(bayesian_regression bayesian_regression.cpp)
target_link_libraries(bayesian_regression PRIVATE fortuna)

# Uncertainty quantification example
add_executable(uncertainty_quantification uncertainty_quantification.cpp)
target_link_libraries(uncertainty_quantification PRIVATE fortuna)

# XOR problem example
add_executable(xor_example xor_example.cpp)
target_link_libraries(xor_example PRIVATE fortuna)
