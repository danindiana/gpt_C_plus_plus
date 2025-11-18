#pragma once

#include "types.hpp"
#include "vector_ops.hpp"
#include "activation.hpp"
#include <memory>
#include <string>
#include <optional>

namespace gru {

/**
 * @brief Modern C++20 implementation of a Gated Recurrent Unit (GRU)
 *
 * A GRU is a type of recurrent neural network (RNN) that uses gating mechanisms
 * to control the flow of information, making it effective for sequence modeling tasks.
 *
 * Key components:
 * - Update gate (z): Controls how much of the previous hidden state to keep
 * - Reset gate (r): Controls how much of the previous hidden state to forget
 * - Candidate activation: Proposes a new hidden state
 *
 * @see https://arxiv.org/abs/1406.1078 (Cho et al., 2014)
 */
class GRU {
public:
    /**
     * @brief Construct a new GRU cell
     *
     * @param input_size Dimension of input vectors
     * @param hidden_size Dimension of hidden state
     * @param use_xavier Use Xavier initialization (recommended) vs random initialization
     */
    explicit GRU(std::size_t input_size, std::size_t hidden_size, bool use_xavier = true);

    /**
     * @brief Perform forward pass through the GRU cell
     *
     * Computes the new hidden state given an input vector:
     * 1. z_t = σ(W_z · [x_t, h_{t-1}] + b_z)  (update gate)
     * 2. r_t = σ(W_r · [x_t, h_{t-1}] + b_r)  (reset gate)
     * 3. h̃_t = tanh(W_h · [x_t, r_t ⊙ h_{t-1}] + b_h)  (candidate)
     * 4. h_t = z_t ⊙ h_{t-1} + (1 - z_t) ⊙ h̃_t  (new hidden state)
     *
     * @param x_t Input vector at time step t
     * @return Vector New hidden state h_t
     * @throws std::invalid_argument if input dimension doesn't match input_size
     */
    [[nodiscard]] Vector forward(const Vector& x_t);

    /**
     * @brief Reset the hidden state to zeros
     */
    void reset_state();

    /**
     * @brief Get the current hidden state
     * @return const Vector& Current hidden state
     */
    [[nodiscard]] const Vector& get_hidden_state() const noexcept { return h_prev_; }

    /**
     * @brief Get input dimension
     */
    [[nodiscard]] std::size_t input_size() const noexcept { return input_size_; }

    /**
     * @brief Get hidden state dimension
     */
    [[nodiscard]] std::size_t hidden_size() const noexcept { return hidden_size_; }

    /**
     * @brief Process a sequence of inputs
     *
     * @param sequence Vector of input vectors
     * @param return_sequences If true, return all hidden states; if false, return only last
     * @return std::vector<Vector> Hidden states
     */
    [[nodiscard]] std::vector<Vector> forward_sequence(
        const std::vector<Vector>& sequence,
        bool return_sequences = false
    );

private:
    std::size_t input_size_;
    std::size_t hidden_size_;

    // Weight matrices for update, reset, and candidate gates
    Matrix Wz_;  // Update gate weights
    Matrix Wr_;  // Reset gate weights
    Matrix Wh_;  // Candidate activation weights

    // Bias vectors
    Vector bz_;  // Update gate bias
    Vector br_;  // Reset gate bias
    Vector bh_;  // Candidate activation bias

    // Hidden state
    Vector h_prev_;  // Previous/current hidden state

    // Validate input dimensions
    void validate_input(const Vector& x) const;
};

} // namespace gru
