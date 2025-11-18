#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <concepts>
#include <memory>

namespace fortuna {

// Type aliases for clarity and flexibility
using Real = double;
using Size = std::size_t;

// Forward declarations
template<typename T>
class Matrix;

template<typename T>
class Tensor;

// Concept for numeric types
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Concept for floating point types
template<typename T>
concept FloatingPoint = std::floating_point<T>;

// Data structures
using RealVector = std::vector<Real>;
using RealMatrix = std::vector<RealVector>;
using Shape = std::vector<Size>;

// Smart pointer aliases
template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

// Bayesian Method Enumeration
enum class BayesianMethod {
    MAP,                // Maximum A Posteriori
    ADVI,               // Automatic Differentiation Variational Inference
    DEEP_ENSEMBLES,     // Deep Ensembles
    LAPLACE_GN,         // Laplace approximation with Gauss-Newton Hessian
    SWAG,               // Stochastic Weight Averaging Gaussian
    SGHMC,              // Stochastic Gradient Hamiltonian Monte Carlo
    CSGDLD,             // Cyclical Stochastic Gradient Langevin Dynamics
    SNNGP               // Spectral-normalized Neural Gaussian Process
};

// Activation function types
enum class ActivationType {
    SIGMOID,
    TANH,
    RELU,
    LEAKY_RELU,
    ELU,
    SOFTMAX,
    LINEAR
};

// Optimizer types
enum class OptimizerType {
    SGD,
    ADAM,
    RMSPROP,
    ADAGRAD
};

// Loss function types
enum class LossType {
    MSE,                // Mean Squared Error
    CROSS_ENTROPY,      // Cross Entropy
    BINARY_CROSS_ENTROPY,
    KL_DIVERGENCE       // KL Divergence
};

} // namespace fortuna
