/**
 * @file basic_gru.cpp
 * @brief Basic GRU usage example demonstrating single-step forward pass
 */

#include "gru/gru.hpp"
#include <iostream>
#include <iomanip>

void print_vector(const std::string& name, const gru::Vector& v) {
    std::cout << name << ": [";
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::cout << std::fixed << std::setprecision(4) << v[i];
        if (i < v.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    std::cout << "=== Basic GRU Example ===\n\n";

    // Create a GRU cell with input size 5 and hidden size 10
    constexpr std::size_t input_size = 5;
    constexpr std::size_t hidden_size = 10;

    std::cout << "Creating GRU cell (input_size=" << input_size
              << ", hidden_size=" << hidden_size << ")\n\n";

    gru::GRU gru_cell(input_size, hidden_size);

    // Create sample input
    gru::Vector input = {1.0, 2.0, 3.0, 4.0, 5.0};

    std::cout << "Processing single input vector:\n";
    print_vector("Input", input);

    // Forward pass
    gru::Vector output = gru_cell.forward(input);

    print_vector("Output (hidden state)", output);

    std::cout << "\nProcessing another input (stateful):\n";
    gru::Vector input2 = {0.5, 1.5, 2.5, 3.5, 4.5};
    print_vector("Input", input2);

    output = gru_cell.forward(input2);
    print_vector("Output (hidden state)", output);

    // Reset state
    std::cout << "\nResetting hidden state...\n";
    gru_cell.reset_state();

    output = gru_cell.forward(input2);
    print_vector("Output after reset", output);

    std::cout << "\nExample completed successfully!\n";
    return 0;
}
