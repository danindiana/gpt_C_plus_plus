#pragma once

#include "types.hpp"
#include "retriever.hpp"
#include "generator.hpp"
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace realm {

/**
 * @brief Retrieval-Augmented Generation (RAG) Model
 *
 * Combines retriever and generator components to perform
 * retrieval-augmented language generation.
 */
class RAGModel {
public:
    /**
     * @brief Construct a RAG model
     * @param retriever Retriever component
     * @param generator Generator component
     * @param hyperparams Hyperparameters
     */
    RAGModel(
        std::shared_ptr<Retriever> retriever,
        std::shared_ptr<Generator> generator,
        const HyperParams& hyperparams
    );

    /**
     * @brief Generate answer for a query
     * @param query Input query
     * @return Generation result
     */
    [[nodiscard]] GenerationResult answer_query(const Query& query);

    /**
     * @brief Generate answer with explicit document retrieval
     * @param query Input query
     * @param num_docs Number of documents to retrieve
     * @return Generation result with retrieved documents
     */
    [[nodiscard]] std::pair<GenerationResult, std::vector<RetrievalResult>>
        answer_with_retrieval(const Query& query, size_t num_docs);

    /**
     * @brief Index documents into the retriever
     * @param documents Documents to index
     */
    void index_documents(const std::vector<Document>& documents);

    /**
     * @brief Get retriever component
     * @return Shared pointer to retriever
     */
    [[nodiscard]] std::shared_ptr<Retriever> get_retriever() const {
        return retriever_;
    }

    /**
     * @brief Get generator component
     * @return Shared pointer to generator
     */
    [[nodiscard]] std::shared_ptr<Generator> get_generator() const {
        return generator_;
    }

    /**
     * @brief Save model to disk
     * @param path Directory path to save model
     * @return True if successful
     */
    bool save(const std::string& path) const;

    /**
     * @brief Load model from disk
     * @param path Directory path to load model from
     * @return Loaded RAG model or nullptr on failure
     */
    static std::unique_ptr<RAGModel> load(const std::string& path);

private:
    std::shared_ptr<Retriever> retriever_;
    std::shared_ptr<Generator> generator_;
    HyperParams hyperparams_;
};

/**
 * @brief RAG Model Trainer
 *
 * Handles end-to-end training of the RAG model.
 */
class RAGTrainer {
public:
    /**
     * @brief Training configuration
     */
    struct TrainingConfig {
        // Whether to train retriever and generator jointly or separately
        bool joint_training{false};

        // Number of pre-training epochs for retriever
        size_t retriever_pretrain_epochs{10};

        // Number of pre-training epochs for generator
        size_t generator_pretrain_epochs{10};

        // Number of joint training epochs
        size_t joint_training_epochs{50};

        // Logging frequency
        size_t log_every_n_steps{100};

        // Validation frequency
        size_t validate_every_n_epochs{5};
    };

    RAGTrainer(
        std::shared_ptr<RAGModel> model,
        const HyperParams& hyperparams,
        const TrainingConfig& training_config
    );

    /**
     * @brief Train the RAG model
     * @param documents Training documents
     * @param queries Training queries
     * @param relevance_labels Relevance labels for queries
     * @return Final training statistics
     */
    TrainingStats train(
        const std::vector<Document>& documents,
        const std::vector<Query>& queries,
        const std::vector<std::vector<DocumentId>>& relevance_labels
    );

    /**
     * @brief Pre-train retriever component
     * @param documents Training documents
     * @param queries Training queries
     * @param relevance_labels Relevance labels
     * @return Training statistics
     */
    TrainingStats pretrain_retriever(
        const std::vector<Document>& documents,
        const std::vector<Query>& queries,
        const std::vector<std::vector<DocumentId>>& relevance_labels
    );

    /**
     * @brief Pre-train generator component
     * @param documents Training documents
     * @return Training statistics
     */
    TrainingStats pretrain_generator(
        const std::vector<Document>& documents
    );

    /**
     * @brief Joint training of retriever and generator
     * @param documents Training documents
     * @param queries Training queries
     * @param relevance_labels Relevance labels
     * @return Training statistics
     */
    TrainingStats joint_train(
        const std::vector<Document>& documents,
        const std::vector<Query>& queries,
        const std::vector<std::vector<DocumentId>>& relevance_labels
    );

    /**
     * @brief Validate model performance
     * @param validation_queries Validation queries
     * @param validation_labels Validation relevance labels
     * @return Validation statistics
     */
    TrainingStats validate(
        const std::vector<Query>& validation_queries,
        const std::vector<std::vector<DocumentId>>& validation_labels
    );

private:
    std::shared_ptr<RAGModel> model_;
    HyperParams hyperparams_;
    TrainingConfig training_config_;

    void log_training_progress(const TrainingStats& stats, size_t step);
};

} // namespace realm
