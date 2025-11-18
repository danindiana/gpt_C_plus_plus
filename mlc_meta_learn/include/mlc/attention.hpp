#pragma once

#include "types.hpp"
#include "tensor.hpp"
#include <memory>
#include <vector>
#include <cmath>

namespace mlc {

/**
 * @brief Multi-head attention mechanism
 *
 * Implements the attention mechanism from "Attention is All You Need"
 * Key component for the MLC network's ability to compositionally generalize
 */
class MultiHeadAttention {
public:
    explicit MultiHeadAttention(std::size_t embed_dim, std::size_t num_heads)
        : embed_dim_(embed_dim)
        , num_heads_(num_heads)
        , head_dim_(embed_dim / num_heads) {

        if (embed_dim % num_heads != 0) {
            throw std::invalid_argument("Embedding dimension must be divisible by number of heads");
        }

        // Initialize weight matrices
        initialize_weights();
    }

    /**
     * @brief Compute scaled dot-product attention
     *
     * Attention(Q, K, V) = softmax(QK^T / sqrt(d_k))V
     */
    [[nodiscard]] Tensor forward(const Tensor& query, const Tensor& key, const Tensor& value) {
        // This is a simplified version - in practice, you'd use proper matrix multiplication
        // and handle batching correctly

        std::size_t seq_len = query.shape()[0];
        Tensor output({seq_len, embed_dim_}, 0.0);

        // Compute attention scores
        std::vector<Float> scores(seq_len * seq_len, 0.0);
        Float scale = 1.0 / std::sqrt(static_cast<Float>(head_dim_));

        // Simplified attention computation
        // In a real implementation, this would involve proper Q, K, V projections
        // and multi-head splitting

        for (std::size_t i = 0; i < seq_len; ++i) {
            Float sum = 0.0;
            for (std::size_t j = 0; j < seq_len; ++j) {
                Float score = compute_score(query, key, i, j) * scale;
                scores[i * seq_len + j] = std::exp(score);
                sum += scores[i * seq_len + j];
            }

            // Normalize scores
            for (std::size_t j = 0; j < seq_len; ++j) {
                scores[i * seq_len + j] /= sum;
            }
        }

        // Apply attention to values
        for (std::size_t i = 0; i < seq_len; ++i) {
            for (std::size_t d = 0; d < embed_dim_; ++d) {
                Float weighted_sum = 0.0;
                for (std::size_t j = 0; j < seq_len; ++j) {
                    if (j < value.shape()[0] && d < value.shape()[1]) {
                        weighted_sum += scores[i * seq_len + j] * value.at(j, d);
                    }
                }
                output.at(i, d) = weighted_sum;
            }
        }

        return output;
    }

    /**
     * @brief Self-attention: Query, Key, and Value are all the same
     */
    [[nodiscard]] Tensor self_attention(const Tensor& input) {
        return forward(input, input, input);
    }

private:
    std::size_t embed_dim_;
    std::size_t num_heads_;
    std::size_t head_dim_;

    Tensor W_q_;  // Query projection
    Tensor W_k_;  // Key projection
    Tensor W_v_;  // Value projection
    Tensor W_o_;  // Output projection

    void initialize_weights() {
        Float init_scale = 1.0 / std::sqrt(static_cast<Float>(embed_dim_));

        W_q_ = Tensor({embed_dim_, embed_dim_}, 0.0);
        W_k_ = Tensor({embed_dim_, embed_dim_}, 0.0);
        W_v_ = Tensor({embed_dim_, embed_dim_}, 0.0);
        W_o_ = Tensor({embed_dim_, embed_dim_}, 0.0);

        W_q_.randomize(-init_scale, init_scale);
        W_k_.randomize(-init_scale, init_scale);
        W_v_.randomize(-init_scale, init_scale);
        W_o_.randomize(-init_scale, init_scale);
    }

    [[nodiscard]] Float compute_score(const Tensor& q, const Tensor& k,
                                       std::size_t i, std::size_t j) const {
        // Simplified dot product for demonstration
        Float score = 0.0;
        std::size_t dim = std::min(q.shape()[1], k.shape()[1]);
        for (std::size_t d = 0; d < dim; ++d) {
            score += q.at(i, d) * k.at(j, d);
        }
        return score;
    }
};

} // namespace mlc
