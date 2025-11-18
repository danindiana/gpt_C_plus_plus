#pragma once

#include "types.hpp"
#include "retriever.hpp"
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace realm {

/**
 * @brief Text generation result
 */
struct GenerationResult {
    std::string generated_text;
    std::vector<std::string> tokens;
    Float confidence_score{0.0f};
    std::vector<DocumentId> used_documents;
};

/**
 * @brief Masked token for MLM training
 */
struct MaskedToken {
    size_t position;
    std::string original_token;
    std::string masked_token{"[MASK]"};
};

/**
 * @brief Generator Component
 *
 * Responsible for generating text using masked language modeling
 * and retrieved documents.
 */
class Generator {
public:
    explicit Generator(
        VectorDim hidden_size,
        const HyperParams& hyperparams
    );

    /**
     * @brief Generate text given a query and retrieved documents
     * @param query Input query
     * @param retrieved_docs Retrieved documents
     * @return Generation result
     */
    [[nodiscard]] GenerationResult generate(
        const Query& query,
        const std::vector<RetrievalResult>& retrieved_docs
    );

    /**
     * @brief Predict masked tokens
     * @param masked_input Input with masked tokens
     * @param context_docs Context documents
     * @return Predicted tokens
     */
    [[nodiscard]] std::vector<std::string> predict_masked_tokens(
        const std::vector<std::string>& masked_input,
        const std::vector<Document>& context_docs
    );

    /**
     * @brief Compute MLM loss for training
     * @param input Input text
     * @param masked_positions Positions of masked tokens
     * @param target_tokens True tokens at masked positions
     * @param context_docs Context documents
     * @return MLM loss value
     */
    [[nodiscard]] Float compute_mlm_loss(
        const std::vector<std::string>& input,
        const std::vector<size_t>& masked_positions,
        const std::vector<std::string>& target_tokens,
        const std::vector<Document>& context_docs
    );

    /**
     * @brief Create masked tokens for training
     * @param tokens Input tokens
     * @param mask_probability Probability of masking each token
     * @return Vector of masked tokens
     */
    [[nodiscard]] std::vector<MaskedToken> create_masked_tokens(
        const std::vector<std::string>& tokens,
        Float mask_probability = 0.15f
    );

private:
    VectorDim hidden_size_;
    HyperParams hyperparams_;

    // In production: neural network layers
    // std::unique_ptr<TransformerModel> model_;

    [[nodiscard]] DenseVector<Float> encode_input(
        const std::vector<std::string>& tokens,
        const std::vector<Document>& context_docs
    );
};

/**
 * @brief Generator Trainer
 *
 * Handles training of the generator component using masked language modeling.
 */
class GeneratorTrainer {
public:
    explicit GeneratorTrainer(
        std::shared_ptr<Generator> generator,
        const HyperParams& hyperparams
    );

    /**
     * @brief Train the generator
     * @param training_data Training documents
     * @param context_documents Context documents for each training sample
     * @return Final training statistics
     */
    TrainingStats train(
        const std::vector<Document>& training_data,
        const std::vector<std::vector<Document>>& context_documents
    );

    /**
     * @brief Train for one epoch
     * @param training_data Training documents
     * @param context_documents Context documents
     * @return Epoch training statistics
     */
    TrainingStats train_epoch(
        const std::vector<Document>& training_data,
        const std::vector<std::vector<Document>>& context_documents
    );

private:
    std::shared_ptr<Generator> generator_;
    HyperParams hyperparams_;

    Float compute_batch_loss(
        const std::vector<Document>& batch_docs,
        const std::vector<std::vector<Document>>& batch_context
    );
};

} // namespace realm
