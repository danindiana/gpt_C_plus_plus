#pragma once

#include "../core/types.hpp"
#include <random>
#include <cmath>
#include <string>

namespace fortuna::utils {

/**
 * @brief Weight initialization strategies for neural networks
 */
class Initializers {
public:
    /**
     * @brief Xavier/Glorot uniform initialization
     *
     * Samples from U(-limit, limit) where limit = sqrt(6 / (fan_in + fan_out))
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param seed Random seed
     */
    static void xavier_uniform(RealMatrix& weights, Size fan_in, Size fan_out,
                               unsigned int seed = std::random_device{}());

    /**
     * @brief Xavier/Glorot normal initialization
     *
     * Samples from N(0, std^2) where std = sqrt(2 / (fan_in + fan_out))
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param seed Random seed
     */
    static void xavier_normal(RealMatrix& weights, Size fan_in, Size fan_out,
                             unsigned int seed = std::random_device{}());

    /**
     * @brief He/Kaiming uniform initialization (good for ReLU)
     *
     * Samples from U(-limit, limit) where limit = sqrt(6 / fan_in)
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param seed Random seed
     */
    static void he_uniform(RealMatrix& weights, Size fan_in, Size fan_out,
                          unsigned int seed = std::random_device{}());

    /**
     * @brief He/Kaiming normal initialization (good for ReLU)
     *
     * Samples from N(0, std^2) where std = sqrt(2 / fan_in)
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param seed Random seed
     */
    static void he_normal(RealMatrix& weights, Size fan_in, Size fan_out,
                         unsigned int seed = std::random_device{}());

    /**
     * @brief LeCun uniform initialization (good for SELU)
     *
     * Samples from U(-limit, limit) where limit = sqrt(3 / fan_in)
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param seed Random seed
     */
    static void lecun_uniform(RealMatrix& weights, Size fan_in, Size fan_out,
                             unsigned int seed = std::random_device{}());

    /**
     * @brief LeCun normal initialization (good for SELU)
     *
     * Samples from N(0, std^2) where std = sqrt(1 / fan_in)
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param seed Random seed
     */
    static void lecun_normal(RealMatrix& weights, Size fan_in, Size fan_out,
                            unsigned int seed = std::random_device{}());

    /**
     * @brief Zero initialization
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     */
    static void zeros(RealMatrix& weights, Size fan_in, Size fan_out);

    /**
     * @brief Constant initialization
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param value Constant value
     */
    static void constant(RealMatrix& weights, Size fan_in, Size fan_out, Real value);

    /**
     * @brief Initialize bias vector to zeros
     *
     * @param biases Output bias vector
     * @param size Size of bias vector
     */
    static void zero_bias(RealVector& biases, Size size);

    /**
     * @brief Initialize weights based on method name
     *
     * @param weights Output weight matrix
     * @param fan_in Number of input units
     * @param fan_out Number of output units
     * @param method Initialization method name
     * @param seed Random seed
     */
    static void initialize(RealMatrix& weights, Size fan_in, Size fan_out,
                          const std::string& method = "xavier",
                          unsigned int seed = std::random_device{}());
};

} // namespace fortuna::utils
