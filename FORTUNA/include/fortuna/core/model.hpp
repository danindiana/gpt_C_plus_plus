#pragma once

#include "types.hpp"
#include "layer.hpp"
#include <vector>
#include <span>
#include <string>
#include <optional>

namespace fortuna::core {

/**
 * @brief Neural network model class
 *
 * This class represents a complete neural network model consisting of
 * multiple layers. It provides methods for forward propagation and
 * model management.
 */
class Model {
public:
    /**
     * @brief Construct a new Model object
     *
     * @param name Optional model name
     */
    explicit Model(std::string name = "FortunaModel");

    /**
     * @brief Add a layer to the model
     *
     * @param layer Layer to add
     */
    void add_layer(const Layer& layer);

    /**
     * @brief Add a layer to the model (move semantics)
     *
     * @param layer Layer to add
     */
    void add_layer(Layer&& layer);

    /**
     * @brief Build the model with specified layer sizes
     *
     * @param layer_sizes Vector of layer sizes [input, hidden1, hidden2, ..., output]
     * @param activation Default activation function for hidden layers
     * @param output_activation Activation function for output layer
     */
    void build(std::span<const Size> layer_sizes,
               ActivationType activation = ActivationType::RELU,
               ActivationType output_activation = ActivationType::LINEAR);

    /**
     * @brief Perform forward pass through the entire model
     *
     * @param inputs Input vector
     * @return RealVector Model output
     */
    [[nodiscard]] RealVector forward(std::span<const Real> inputs) const;

    /**
     * @brief Perform forward pass and return all intermediate activations
     *
     * @param inputs Input vector
     * @return std::vector<RealVector> All layer outputs
     */
    [[nodiscard]] std::vector<RealVector> forward_with_activations(
        std::span<const Real> inputs) const;

    /**
     * @brief Get number of layers
     *
     * @return Size Number of layers in the model
     */
    [[nodiscard]] Size num_layers() const noexcept { return layers_.size(); }

    /**
     * @brief Get a specific layer
     *
     * @param index Layer index
     * @return const Layer& Reference to the layer
     */
    [[nodiscard]] const Layer& get_layer(Size index) const;

    /**
     * @brief Get mutable reference to a specific layer
     *
     * @param index Layer index
     * @return Layer& Reference to the layer
     */
    [[nodiscard]] Layer& get_layer(Size index);

    /**
     * @brief Get all layers
     *
     * @return const std::vector<Layer>& Reference to all layers
     */
    [[nodiscard]] const std::vector<Layer>& layers() const noexcept { return layers_; }

    /**
     * @brief Get total number of parameters in the model
     *
     * @return Size Total parameters across all layers
     */
    [[nodiscard]] Size num_parameters() const noexcept;

    /**
     * @brief Initialize all layer weights
     *
     * @param method Initialization method
     */
    void initialize_weights(const std::string& method = "xavier");

    /**
     * @brief Get model name
     *
     * @return const std::string& Model name
     */
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    /**
     * @brief Set model name
     *
     * @param name New model name
     */
    void set_name(std::string name) { name_ = std::move(name); }

    /**
     * @brief Get model summary as a string
     *
     * @return std::string Model architecture summary
     */
    [[nodiscard]] std::string summary() const;

private:
    std::vector<Layer> layers_;
    std::string name_;

    /**
     * @brief Validate model architecture
     *
     * @return bool True if model is valid
     */
    [[nodiscard]] bool validate() const;
};

} // namespace fortuna::core
