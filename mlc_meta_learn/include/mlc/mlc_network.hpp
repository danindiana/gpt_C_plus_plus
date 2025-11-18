#pragma once

#include "types.hpp"
#include "example.hpp"
#include "episode.hpp"
#include "tensor.hpp"
#include "attention.hpp"
#include <memory>
#include <unordered_map>
#include <random>
#include <optional>

namespace mlc {

/**
 * @brief Meta-Learning for Compositionality Network
 *
 * Implements the MLC framework from Lake & Baroni (2023)
 * Key features:
 * - Episode-based meta-learning
 * - Attention mechanisms for compositional generalization
 * - Few-shot learning capabilities
 */
class MLCNetwork {
public:
    explicit MLCNetwork(const MLCConfig& config = MLCConfig{})
        : config_(config)
        , rng_(config.random_seed) {

        if (!config_.is_valid()) {
            throw std::invalid_argument("Invalid MLC configuration");
        }

        initialize_network();
        initialize_vocabulary();
    }

    /**
     * @brief Train the network on a single episode
     *
     * @param episode The training episode containing study and query examples
     * @return Loss value for this episode
     */
    Float train_on_episode(const Episode& episode) {
        if (!episode.is_valid()) {
            throw std::invalid_argument("Invalid episode");
        }

        stats_.total_examples++;

        // Process study examples to build context
        auto context = encode_study_examples(episode.study_examples());

        // Process query with attention to study examples
        auto predicted_output = predict_with_context(
            episode.query_example().instruction(),
            context
        );

        // Compute loss and update
        Float loss = compute_loss(predicted_output, episode.query_example().output());

        if (sequences_match(predicted_output, episode.query_example().output())) {
            stats_.correct_predictions++;
        }

        stats_.loss += loss;
        stats_.update_accuracy();

        // Gradient descent update (simplified)
        update_weights(loss);

        return loss;
    }

    /**
     * @brief Evaluate the network on a set of episodes
     *
     * @param episodes Episodes to evaluate on
     * @return Evaluation accuracy
     */
    Float evaluate(const std::vector<Episode>& episodes) {
        if (episodes.empty()) {
            return 0.0;
        }

        std::size_t correct = 0;
        Float total_loss = 0.0;

        for (const auto& episode : episodes) {
            if (!episode.is_valid()) continue;

            auto context = encode_study_examples(episode.study_examples());
            auto predicted = predict_with_context(
                episode.query_example().instruction(),
                context
            );

            if (sequences_match(predicted, episode.query_example().output())) {
                correct++;
            }

            total_loss += compute_loss(predicted, episode.query_example().output());
        }

        stats_.validation_accuracy = static_cast<Float>(correct) / episodes.size();
        stats_.validation_loss = total_loss / episodes.size();

        return stats_.validation_accuracy;
    }

    /**
     * @brief Predict output for a given instruction with context
     *
     * @param instruction The instruction to execute
     * @param study_examples Examples providing context
     * @return Predicted output sequence
     */
    [[nodiscard]] OutputSequence predict(
        const Instruction& instruction,
        const std::vector<Example>& study_examples = {}
    ) {
        auto context = encode_study_examples(study_examples);
        return predict_with_context(instruction, context);
    }

    // Accessors
    [[nodiscard]] const TrainingStats& stats() const noexcept {
        return stats_;
    }

    [[nodiscard]] const MLCConfig& config() const noexcept {
        return config_;
    }

    void reset_stats() noexcept {
        stats_.reset();
    }

private:
    MLCConfig config_;
    TrainingStats stats_;
    std::mt19937 rng_;

    // Network components
    std::unique_ptr<MultiHeadAttention> attention_;

    // Embeddings
    std::unordered_map<std::string, std::size_t> vocabulary_;
    Tensor embedding_matrix_;

    // Hidden state
    Tensor hidden_state_;

    // Output projection
    Tensor output_weights_;

    void initialize_network() {
        // Initialize attention mechanism
        if (config_.use_attention) {
            attention_ = std::make_unique<MultiHeadAttention>(
                config_.embedding_dim,
                config_.attention_heads
            );
        }

        // Initialize embedding matrix
        embedding_matrix_ = Tensor(
            {1000, config_.embedding_dim},  // Vocab size × embedding dim
            0.0
        );
        embedding_matrix_.randomize(-0.1, 0.1);

        // Initialize hidden state
        hidden_state_ = Tensor({config_.hidden_size}, 0.0);

        // Initialize output weights
        output_weights_ = Tensor({config_.hidden_size, config_.embedding_dim}, 0.0);
        output_weights_.randomize(-0.1, 0.1);
    }

    void initialize_vocabulary() {
        // Add common tokens
        vocabulary_["<PAD>"] = 0;
        vocabulary_["<UNK>"] = 1;
        vocabulary_["<START>"] = 2;
        vocabulary_["<END>"] = 3;

        // These would normally be learned from data
        std::vector<std::string> common_tokens = {
            "jump", "skip", "tiptoe", "run", "walk",
            "twice", "thrice", "once",
            "circle", "square", "triangle", "diamond",
            "left", "right", "around", "and", "then"
        };

        std::size_t idx = 4;
        for (const auto& token : common_tokens) {
            vocabulary_[token] = idx++;
        }
    }

    [[nodiscard]] Tensor encode_study_examples(const std::vector<Example>& examples) {
        if (examples.empty()) {
            return Tensor({1, config_.embedding_dim}, 0.0);
        }

        // Encode each example and aggregate
        std::vector<Tensor> encoded;
        encoded.reserve(examples.size());

        for (const auto& ex : examples) {
            encoded.push_back(encode_example(ex));
        }

        // Use attention to aggregate if enabled
        if (config_.use_attention && attention_ && encoded.size() > 1) {
            // Stack tensors and apply attention
            Tensor stacked({encoded.size(), config_.embedding_dim}, 0.0);
            for (std::size_t i = 0; i < encoded.size(); ++i) {
                for (std::size_t j = 0; j < config_.embedding_dim; ++j) {
                    stacked.at(i, j) = encoded[i][j];
                }
            }
            return attention_->self_attention(stacked);
        }

        // Simple averaging as fallback
        Tensor result({1, config_.embedding_dim}, 0.0);
        for (const auto& enc : encoded) {
            result += enc;
        }
        result *= (1.0 / encoded.size());

        return result;
    }

    [[nodiscard]] Tensor encode_example(const Example& example) {
        // Simple bag-of-words encoding
        Tensor encoding({1, config_.embedding_dim}, 0.0);

        // Encode instruction
        auto tokens = tokenize(example.instruction());
        for (const auto& token : tokens) {
            auto it = vocabulary_.find(token);
            std::size_t idx = (it != vocabulary_.end()) ? it->second : vocabulary_["<UNK>"];

            // Add embedding for this token
            if (idx < embedding_matrix_.shape()[0]) {
                for (std::size_t i = 0; i < config_.embedding_dim; ++i) {
                    encoding[i] += embedding_matrix_.at(idx, i);
                }
            }
        }

        // Normalize
        Float norm = std::sqrt(encoding.sum());
        if (norm > 0.0) {
            encoding *= (1.0 / norm);
        }

        return encoding;
    }

    [[nodiscard]] OutputSequence predict_with_context(
        const Instruction& instruction,
        const Tensor& context
    ) {
        // Encode instruction
        auto inst_encoding = encode_instruction(instruction);

        // Ensure context and instruction encoding have compatible shapes
        Tensor combined({1, config_.embedding_dim}, 0.0);

        // Add instruction encoding
        for (std::size_t i = 0; i < config_.embedding_dim && i < inst_encoding.size(); ++i) {
            combined[i] = inst_encoding[i];
        }

        // Add context if available
        if (context.size() > 0) {
            for (std::size_t i = 0; i < config_.embedding_dim && i < context.size(); ++i) {
                combined[i] += context[i];
            }
        }

        // Apply nonlinearity
        combined = activation::tanh(combined);

        // Decode to output sequence
        return decode_to_sequence(combined);
    }

    [[nodiscard]] Tensor encode_instruction(const Instruction& instruction) {
        Tensor encoding({1, config_.embedding_dim}, 0.0);

        auto tokens = tokenize(instruction);
        for (const auto& token : tokens) {
            auto it = vocabulary_.find(token);
            std::size_t idx = (it != vocabulary_.end()) ? it->second : vocabulary_["<UNK>"];

            if (idx < embedding_matrix_.shape()[0]) {
                for (std::size_t i = 0; i < config_.embedding_dim; ++i) {
                    encoding[i] += embedding_matrix_.at(idx, i);
                }
            }
        }

        return encoding;
    }

    [[nodiscard]] OutputSequence decode_to_sequence(const Tensor& encoding) {
        // Simplified decoding - in practice, use beam search or similar
        OutputSequence result;

        // Map encoding to vocabulary items
        Float max_similarity = -std::numeric_limits<Float>::infinity();
        std::string best_token;

        for (const auto& [token, idx] : vocabulary_) {
            if (token[0] == '<') continue;  // Skip special tokens

            Float similarity = 0.0;
            if (idx < embedding_matrix_.shape()[0]) {
                for (std::size_t i = 0; i < config_.embedding_dim; ++i) {
                    similarity += encoding[i] * embedding_matrix_.at(idx, i);
                }
            }

            if (similarity > max_similarity) {
                max_similarity = similarity;
                best_token = token;
            }
        }

        if (!best_token.empty()) {
            result.push_back(best_token);
        }

        return result;
    }

    [[nodiscard]] std::vector<std::string> tokenize(const std::string& text) const {
        std::vector<std::string> tokens;
        std::string current;

        for (char c : text) {
            if (std::isspace(c)) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }

        if (!current.empty()) {
            tokens.push_back(current);
        }

        return tokens;
    }

    [[nodiscard]] Float compute_loss(
        const OutputSequence& predicted,
        const OutputSequence& target
    ) const {
        // Simple 0-1 loss for now
        return sequences_match(predicted, target) ? 0.0 : 1.0;
    }

    [[nodiscard]] bool sequences_match(
        const OutputSequence& a,
        const OutputSequence& b
    ) const {
        return a == b;
    }

    void update_weights(Float loss) {
        // Simplified gradient descent update
        // In practice, use proper backpropagation
        Float lr = config_.learning_rate;

        // Update embeddings with small random perturbation proportional to loss
        if (loss > 0.0) {
            std::uniform_real_distribution<Float> dist(-lr * loss, lr * loss);
            for (auto& val : embedding_matrix_.data()) {
                val += dist(rng_);
            }
        }
    }
};

} // namespace mlc
