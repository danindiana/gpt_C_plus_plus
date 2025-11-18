#include "fortuna/core/model.hpp"
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace fortuna::core {

Model::Model(std::string name) : name_(std::move(name)) {}

void Model::add_layer(const Layer& layer) {
    if (!layers_.empty()) {
        // Validate that layer dimensions match
        const auto& last_layer = layers_.back();
        if (last_layer.output_size() != layer.input_size()) {
            std::ostringstream oss;
            oss << "Layer dimension mismatch: previous layer output size ("
                << last_layer.output_size() << ") != new layer input size ("
                << layer.input_size() << ")";
            throw std::invalid_argument(oss.str());
        }
    }
    layers_.push_back(layer);
}

void Model::add_layer(Layer&& layer) {
    if (!layers_.empty()) {
        const auto& last_layer = layers_.back();
        if (last_layer.output_size() != layer.input_size()) {
            std::ostringstream oss;
            oss << "Layer dimension mismatch: previous layer output size ("
                << last_layer.output_size() << ") != new layer input size ("
                << layer.input_size() << ")";
            throw std::invalid_argument(oss.str());
        }
    }
    layers_.push_back(std::move(layer));
}

void Model::build(std::span<const Size> layer_sizes,
                  ActivationType activation,
                  ActivationType output_activation) {
    if (layer_sizes.size() < 2) {
        throw std::invalid_argument("Need at least input and output layer sizes");
    }

    layers_.clear();

    // Add hidden layers
    for (Size i = 0; i < layer_sizes.size() - 2; ++i) {
        std::string layer_name = "hidden_" + std::to_string(i);
        layers_.emplace_back(layer_sizes[i], layer_sizes[i + 1],
                           activation, std::move(layer_name));
    }

    // Add output layer
    Size last_idx = layer_sizes.size() - 2;
    layers_.emplace_back(layer_sizes[last_idx], layer_sizes[last_idx + 1],
                        output_activation, "output");
}

RealVector Model::forward(std::span<const Real> inputs) const {
    if (layers_.empty()) {
        throw std::runtime_error("Model has no layers");
    }

    RealVector current_output(inputs.begin(), inputs.end());

    for (const auto& layer : layers_) {
        current_output = layer.forward(current_output);
    }

    return current_output;
}

std::vector<RealVector> Model::forward_with_activations(std::span<const Real> inputs) const {
    if (layers_.empty()) {
        throw std::runtime_error("Model has no layers");
    }

    std::vector<RealVector> activations;
    activations.reserve(layers_.size() + 1);

    // Store input as first activation
    activations.emplace_back(inputs.begin(), inputs.end());

    RealVector current_output(inputs.begin(), inputs.end());

    for (const auto& layer : layers_) {
        current_output = layer.forward(current_output);
        activations.push_back(current_output);
    }

    return activations;
}

const Layer& Model::get_layer(Size index) const {
    if (index >= layers_.size()) {
        std::ostringstream oss;
        oss << "Layer index out of bounds: " << index << " >= " << layers_.size();
        throw std::out_of_range(oss.str());
    }
    return layers_[index];
}

Layer& Model::get_layer(Size index) {
    if (index >= layers_.size()) {
        std::ostringstream oss;
        oss << "Layer index out of bounds: " << index << " >= " << layers_.size();
        throw std::out_of_range(oss.str());
    }
    return layers_[index];
}

Size Model::num_parameters() const noexcept {
    Size total = 0;
    for (const auto& layer : layers_) {
        total += layer.num_parameters();
    }
    return total;
}

void Model::initialize_weights(const std::string& method) {
    for (auto& layer : layers_) {
        layer.initialize_weights(method);
    }
}

std::string Model::summary() const {
    std::ostringstream oss;
    oss << "Model: " << name_ << "\n";
    oss << "________________________________________________________________\n";
    oss << "Layer (type)                Output Shape              Param #\n";
    oss << "================================================================\n";

    Size total_params = 0;
    for (Size i = 0; i < layers_.size(); ++i) {
        const auto& layer = layers_[i];
        Size layer_params = layer.num_parameters();
        total_params += layer_params;

        oss << layer.name() << " (" << i << ")"
            << std::string(20 - layer.name().length(), ' ')
            << "(" << layer.output_size() << ")"
            << std::string(20, ' ')
            << layer_params << "\n";
    }

    oss << "================================================================\n";
    oss << "Total params: " << total_params << "\n";
    oss << "________________________________________________________________\n";

    return oss.str();
}

bool Model::validate() const {
    if (layers_.empty()) {
        return false;
    }

    for (Size i = 1; i < layers_.size(); ++i) {
        if (layers_[i - 1].output_size() != layers_[i].input_size()) {
            return false;
        }
    }

    return true;
}

} // namespace fortuna::core
