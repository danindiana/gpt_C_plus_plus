#include "fortuna/core/layer.hpp"
#include "fortuna/utils/activations.hpp"
#include "fortuna/utils/initializers.hpp"
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace fortuna::core {

Layer::Layer(Size input_size, Size output_size, ActivationType activation, std::string name)
    : input_size_(input_size),
      output_size_(output_size),
      activation_(activation),
      name_(std::move(name)) {

    if (input_size == 0 || output_size == 0) {
        throw std::invalid_argument("Layer dimensions must be positive");
    }

    // Initialize weights matrix [input_size][output_size]
    weights_.resize(input_size);
    for (auto& row : weights_) {
        row.resize(output_size);
    }

    // Initialize biases vector [output_size]
    biases_.resize(output_size, 0.0);

    // Initialize with default method
    initialize_weights("xavier");
}

RealVector Layer::forward(std::span<const Real> inputs) const {
    if (inputs.size() != input_size_) {
        std::ostringstream oss;
        oss << "Input size mismatch: expected " << input_size_
            << ", got " << inputs.size();
        throw std::invalid_argument(oss.str());
    }

    // Compute weighted sum: output = W^T * input + b
    RealVector output(output_size_, 0.0);

    for (Size j = 0; j < output_size_; ++j) {
        Real sum = biases_[j];
        for (Size i = 0; i < input_size_; ++i) {
            sum += inputs[i] * weights_[i][j];
        }
        output[j] = sum;
    }

    // Apply activation function
    return apply_activation(output);
}

RealVector Layer::forward_linear(std::span<const Real> inputs) const {
    if (inputs.size() != input_size_) {
        std::ostringstream oss;
        oss << "Input size mismatch: expected " << input_size_
            << ", got " << inputs.size();
        throw std::invalid_argument(oss.str());
    }

    // Compute weighted sum without activation
    RealVector output(output_size_, 0.0);

    for (Size j = 0; j < output_size_; ++j) {
        Real sum = biases_[j];
        for (Size i = 0; i < input_size_; ++i) {
            sum += inputs[i] * weights_[i][j];
        }
        output[j] = sum;
    }

    return output;
}

void Layer::initialize_weights(const std::string& method) {
    utils::Initializers::initialize(weights_, input_size_, output_size_, method);
    utils::Initializers::zero_bias(biases_, output_size_);
}

void Layer::set_weights(const RealMatrix& weights) {
    if (weights.size() != input_size_) {
        throw std::invalid_argument("Weight matrix row size mismatch");
    }
    for (const auto& row : weights) {
        if (row.size() != output_size_) {
            throw std::invalid_argument("Weight matrix column size mismatch");
        }
    }
    weights_ = weights;
}

void Layer::set_biases(const RealVector& biases) {
    if (biases.size() != output_size_) {
        throw std::invalid_argument("Bias vector size mismatch");
    }
    biases_ = biases;
}

Real Layer::apply_activation(Real x) const {
    return utils::Activations::apply(x, activation_);
}

RealVector Layer::apply_activation(const RealVector& values) const {
    return utils::Activations::apply(values, activation_);
}

} // namespace fortuna::core
