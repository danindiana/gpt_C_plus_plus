#pragma once

#include <concepts>
#include <string>
#include <vector>
#include <memory>
#include <span>
#include <ranges>

namespace mlc {

// Version information
inline constexpr const char* VERSION = "1.0.0";
inline constexpr int VERSION_MAJOR = 1;
inline constexpr int VERSION_MINOR = 0;
inline constexpr int VERSION_PATCH = 0;

// Type aliases for clarity
using Instruction = std::string;
using OutputSequence = std::vector<std::string>;
using Float = double;

// Concept for numeric types
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// Concept for container types
template<typename T>
concept Container = requires(T t) {
    { std::ranges::begin(t) } -> std::input_or_output_iterator;
    { std::ranges::end(t) } -> std::input_or_output_iterator;
    { std::ranges::size(t) } -> std::convertible_to<std::size_t>;
};

// Concept for trainable models
template<typename T>
concept Trainable = requires(T t) {
    { t.train() } -> std::same_as<void>;
    { t.evaluate() } -> Numeric;
};

// Configuration structure for MLC
struct MLCConfig {
    std::size_t hidden_size = 128;
    std::size_t num_layers = 2;
    std::size_t embedding_dim = 64;
    std::size_t attention_heads = 4;
    Float learning_rate = 0.001;
    Float dropout_rate = 0.1;
    std::size_t batch_size = 32;
    std::size_t max_sequence_length = 100;
    bool use_attention = true;
    std::size_t random_seed = 42;

    // Validation
    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return hidden_size > 0 &&
               num_layers > 0 &&
               embedding_dim > 0 &&
               attention_heads > 0 &&
               learning_rate > 0.0 &&
               dropout_rate >= 0.0 && dropout_rate < 1.0 &&
               batch_size > 0 &&
               max_sequence_length > 0;
    }
};

// Training statistics
struct TrainingStats {
    std::size_t epoch = 0;
    Float loss = 0.0;
    Float accuracy = 0.0;
    Float validation_loss = 0.0;
    Float validation_accuracy = 0.0;
    std::size_t total_examples = 0;
    std::size_t correct_predictions = 0;

    void reset() noexcept {
        epoch = 0;
        loss = 0.0;
        accuracy = 0.0;
        validation_loss = 0.0;
        validation_accuracy = 0.0;
        total_examples = 0;
        correct_predictions = 0;
    }

    void update_accuracy() noexcept {
        if (total_examples > 0) {
            accuracy = static_cast<Float>(correct_predictions) / total_examples;
        }
    }
};

} // namespace mlc
