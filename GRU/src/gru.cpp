#include "gru/gru.hpp"
#include <stdexcept>
#include <sstream>

namespace gru {

GRU::GRU(std::size_t input_size, std::size_t hidden_size, bool use_xavier)
    : input_size_(input_size)
    , hidden_size_(hidden_size)
    , h_prev_(zeros(hidden_size))
{
    const std::size_t combined_size = input_size + hidden_size;

    if (use_xavier) {
        // Xavier/Glorot initialization for better gradient flow
        Wz_ = xavier_init(hidden_size, combined_size);
        Wr_ = xavier_init(hidden_size, combined_size);
        Wh_ = xavier_init(hidden_size, combined_size);
    } else {
        // Fallback to random initialization
        Wz_ = Matrix(hidden_size, Vector(combined_size));
        Wr_ = Matrix(hidden_size, Vector(combined_size));
        Wh_ = Matrix(hidden_size, Vector(combined_size));

        for (auto& row : Wz_) row = random_vector(combined_size);
        for (auto& row : Wr_) row = random_vector(combined_size);
        for (auto& row : Wh_) row = random_vector(combined_size);
    }

    // Initialize biases to small values
    bz_ = random_vector(hidden_size, 0.0, 0.01);
    br_ = random_vector(hidden_size, 0.0, 0.01);
    bh_ = random_vector(hidden_size, 0.0, 0.01);
}

Vector GRU::forward(const Vector& x_t) {
    validate_input(x_t);

    // Combine input and previous hidden state: [x_t, h_{t-1}]
    const Vector combined = concatenate(x_t, h_prev_);

    // Update gate: z_t = σ(W_z · [x_t, h_{t-1}] + b_z)
    const Vector z_t = sigmoid(add(matmul(Wz_, combined), bz_));

    // Reset gate: r_t = σ(W_r · [x_t, h_{t-1}] + b_r)
    const Vector r_t = sigmoid(add(matmul(Wr_, combined), br_));

    // Candidate activation: h̃_t = tanh(W_h · [x_t, r_t ⊙ h_{t-1}] + b_h)
    const Vector reset_hidden = hadamard(r_t, h_prev_);
    const Vector combined_reset = concatenate(x_t, reset_hidden);
    const Vector h_tilde = tanh_activation(add(matmul(Wh_, combined_reset), bh_));

    // New hidden state: h_t = z_t ⊙ h_{t-1} + (1 - z_t) ⊙ h̃_t
    const Vector z_complement = complement(z_t);
    const Vector term1 = hadamard(z_t, h_prev_);
    const Vector term2 = hadamard(z_complement, h_tilde);
    h_prev_ = add(term1, term2);

    return h_prev_;
}

void GRU::reset_state() {
    h_prev_ = zeros(hidden_size_);
}

std::vector<Vector> GRU::forward_sequence(
    const std::vector<Vector>& sequence,
    bool return_sequences
) {
    if (sequence.empty()) {
        return {};
    }

    std::vector<Vector> outputs;

    if (return_sequences) {
        outputs.reserve(sequence.size());
        for (const auto& input : sequence) {
            outputs.push_back(forward(input));
        }
    } else {
        // Only return the last output
        for (const auto& input : sequence) {
            (void)forward(input);  // Explicitly discard return value
        }
        outputs.push_back(h_prev_);
    }

    return outputs;
}

void GRU::validate_input(const Vector& x) const {
    if (x.size() != input_size_) {
        std::ostringstream oss;
        oss << "Input dimension mismatch: expected " << input_size_
            << ", got " << x.size();
        throw std::invalid_argument(oss.str());
    }
}

} // namespace gru
