#pragma once

#include "../core/types.hpp"
#include <cmath>
#include <algorithm>
#include <vector>

namespace fortuna::utils {

/**
 * @brief Activation functions for neural networks
 */
class Activations {
public:
    /**
     * @brief Sigmoid activation function
     *
     * @param x Input value
     * @return Real sigmoid(x) = 1 / (1 + exp(-x))
     */
    [[nodiscard]] static constexpr Real sigmoid(Real x) noexcept {
        return 1.0 / (1.0 + std::exp(-x));
    }

    /**
     * @brief Hyperbolic tangent activation function
     *
     * @param x Input value
     * @return Real tanh(x)
     */
    [[nodiscard]] static Real tanh(Real x) noexcept {
        return std::tanh(x);
    }

    /**
     * @brief ReLU (Rectified Linear Unit) activation function
     *
     * @param x Input value
     * @return Real max(0, x)
     */
    [[nodiscard]] static constexpr Real relu(Real x) noexcept {
        return std::max(Real{0.0}, x);
    }

    /**
     * @brief Leaky ReLU activation function
     *
     * @param x Input value
     * @param alpha Slope for negative values (default: 0.01)
     * @return Real x if x > 0, else alpha * x
     */
    [[nodiscard]] static constexpr Real leaky_relu(Real x, Real alpha = 0.01) noexcept {
        return x > 0.0 ? x : alpha * x;
    }

    /**
     * @brief ELU (Exponential Linear Unit) activation function
     *
     * @param x Input value
     * @param alpha Alpha parameter (default: 1.0)
     * @return Real x if x > 0, else alpha * (exp(x) - 1)
     */
    [[nodiscard]] static Real elu(Real x, Real alpha = 1.0) noexcept {
        return x > 0.0 ? x : alpha * (std::exp(x) - 1.0);
    }

    /**
     * @brief Linear (identity) activation function
     *
     * @param x Input value
     * @return Real x
     */
    [[nodiscard]] static constexpr Real linear(Real x) noexcept {
        return x;
    }

    /**
     * @brief Softmax activation function (for vectors)
     *
     * @param x Input vector
     * @return RealVector Softmax output
     */
    [[nodiscard]] static RealVector softmax(const RealVector& x);

    /**
     * @brief Apply activation function based on type
     *
     * @param x Input value
     * @param type Activation type
     * @return Real Activated value
     */
    [[nodiscard]] static Real apply(Real x, ActivationType type);

    /**
     * @brief Apply activation function to vector
     *
     * @param x Input vector
     * @param type Activation type
     * @return RealVector Activated vector
     */
    [[nodiscard]] static RealVector apply(const RealVector& x, ActivationType type);

    /**
     * @brief Derivative of sigmoid function
     *
     * @param x Input value
     * @return Real sigmoid'(x)
     */
    [[nodiscard]] static Real sigmoid_derivative(Real x) noexcept {
        Real sig = sigmoid(x);
        return sig * (1.0 - sig);
    }

    /**
     * @brief Derivative of tanh function
     *
     * @param x Input value
     * @return Real tanh'(x)
     */
    [[nodiscard]] static Real tanh_derivative(Real x) noexcept {
        Real t = tanh(x);
        return 1.0 - t * t;
    }

    /**
     * @brief Derivative of ReLU function
     *
     * @param x Input value
     * @return Real relu'(x)
     */
    [[nodiscard]] static constexpr Real relu_derivative(Real x) noexcept {
        return x > 0.0 ? 1.0 : 0.0;
    }

    /**
     * @brief Derivative of Leaky ReLU function
     *
     * @param x Input value
     * @param alpha Slope parameter
     * @return Real leaky_relu'(x)
     */
    [[nodiscard]] static constexpr Real leaky_relu_derivative(Real x, Real alpha = 0.01) noexcept {
        return x > 0.0 ? 1.0 : alpha;
    }
};

} // namespace fortuna::utils
