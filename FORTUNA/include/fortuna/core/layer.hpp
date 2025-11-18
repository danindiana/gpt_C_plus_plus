#pragma once

#include "types.hpp"
#include <vector>
#include <span>
#include <optional>
#include <string>

namespace fortuna::core {

/**
 * @brief Represents a single layer in a neural network
 *
 * This class encapsulates the weights, biases, and operations for a single
 * layer in a neural network, following modern C++20 design patterns.
 */
class Layer {
public:
    /**
     * @brief Construct a new Layer object
     *
     * @param input_size Number of input neurons
     * @param output_size Number of output neurons
     * @param activation Activation function type
     * @param name Optional layer name
     */
    Layer(Size input_size, Size output_size,
          ActivationType activation = ActivationType::RELU,
          std::string name = "");

    /**
     * @brief Perform forward pass through the layer
     *
     * @param inputs Input vector
     * @return RealVector Output after applying weights, biases, and activation
     */
    [[nodiscard]] RealVector forward(std::span<const Real> inputs) const;

    /**
     * @brief Perform forward pass without activation (useful for output layers)
     *
     * @param inputs Input vector
     * @return RealVector Linear output
     */
    [[nodiscard]] RealVector forward_linear(std::span<const Real> inputs) const;

    /**
     * @brief Initialize weights using specified method
     *
     * @param method Initialization method (Xavier, He, etc.)
     */
    void initialize_weights(const std::string& method = "xavier");

    /**
     * @brief Get layer weights
     *
     * @return const RealMatrix& Reference to weights
     */
    [[nodiscard]] const RealMatrix& get_weights() const noexcept { return weights_; }

    /**
     * @brief Get layer biases
     *
     * @return const RealVector& Reference to biases
     */
    [[nodiscard]] const RealVector& get_biases() const noexcept { return biases_; }

    /**
     * @brief Set layer weights
     *
     * @param weights New weights
     */
    void set_weights(const RealMatrix& weights);

    /**
     * @brief Set layer biases
     *
     * @param biases New biases
     */
    void set_biases(const RealVector& biases);

    /**
     * @brief Get input size
     *
     * @return Size Number of input neurons
     */
    [[nodiscard]] Size input_size() const noexcept { return input_size_; }

    /**
     * @brief Get output size
     *
     * @return Size Number of output neurons
     */
    [[nodiscard]] Size output_size() const noexcept { return output_size_; }

    /**
     * @brief Get activation type
     *
     * @return ActivationType The activation function used
     */
    [[nodiscard]] ActivationType activation_type() const noexcept { return activation_; }

    /**
     * @brief Get layer name
     *
     * @return const std::string& Layer name
     */
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    /**
     * @brief Get total number of parameters in this layer
     *
     * @return Size Total parameters (weights + biases)
     */
    [[nodiscard]] Size num_parameters() const noexcept {
        return input_size_ * output_size_ + output_size_;
    }

private:
    Size input_size_;
    Size output_size_;
    RealMatrix weights_;        // [input_size][output_size]
    RealVector biases_;         // [output_size]
    ActivationType activation_;
    std::string name_;

    /**
     * @brief Apply activation function
     *
     * @param x Input value
     * @return Real Activated value
     */
    [[nodiscard]] Real apply_activation(Real x) const;

    /**
     * @brief Apply activation function to vector
     *
     * @param values Input vector
     * @return RealVector Activated vector
     */
    [[nodiscard]] RealVector apply_activation(const RealVector& values) const;
};

} // namespace fortuna::core
