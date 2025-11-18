/**
 * @file sequence_processing.cpp
 * @brief Demonstrates processing sequences with GRU
 */

#include "gru/gru.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

void print_sequence(const std::string& name, const std::vector<gru::Vector>& seq) {
    std::cout << name << ":\n";
    for (std::size_t t = 0; t < seq.size(); ++t) {
        std::cout << "  t=" << t << ": [";
        for (std::size_t i = 0; i < seq[t].size() && i < 5; ++i) {
            std::cout << std::fixed << std::setprecision(4) << seq[t][i];
            if (i < seq[t].size() - 1 && i < 4) std::cout << ", ";
        }
        if (seq[t].size() > 5) std::cout << ", ...";
        std::cout << "]\n";
    }
}

// Generate a simple sine wave sequence
std::vector<gru::Vector> generate_sine_sequence(std::size_t length, std::size_t features = 1) {
    std::vector<gru::Vector> sequence;
    sequence.reserve(length);

    for (std::size_t t = 0; t < length; ++t) {
        gru::Vector v(features);
        for (std::size_t i = 0; i < features; ++i) {
            v[i] = std::sin(2.0 * M_PI * static_cast<double>(t + i) / 10.0);
        }
        sequence.push_back(v);
    }

    return sequence;
}

int main() {
    std::cout << "=== Sequence Processing Example ===\n\n";

    // Generate a sine wave sequence
    constexpr std::size_t sequence_length = 10;
    constexpr std::size_t input_features = 3;
    constexpr std::size_t hidden_size = 8;

    std::cout << "Generating sine wave sequence (length=" << sequence_length
              << ", features=" << input_features << ")\n\n";

    auto input_sequence = generate_sine_sequence(sequence_length, input_features);

    // Create GRU
    gru::GRU gru_cell(input_features, hidden_size);

    // Process sequence and return all states
    std::cout << "Processing sequence (return all hidden states):\n\n";
    auto all_outputs = gru_cell.forward_sequence(input_sequence, true);

    print_sequence("Hidden states", all_outputs);

    // Reset and process again, returning only last state
    std::cout << "\nProcessing sequence (return only last hidden state):\n\n";
    gru_cell.reset_state();
    auto last_output = gru_cell.forward_sequence(input_sequence, false);

    std::cout << "Last hidden state: [";
    for (std::size_t i = 0; i < last_output[0].size() && i < 8; ++i) {
        std::cout << std::fixed << std::setprecision(4) << last_output[0][i];
        if (i < last_output[0].size() - 1 && i < 7) std::cout << ", ";
    }
    if (last_output[0].size() > 8) std::cout << ", ...";
    std::cout << "]\n";

    std::cout << "\nExample completed successfully!\n";
    return 0;
}
