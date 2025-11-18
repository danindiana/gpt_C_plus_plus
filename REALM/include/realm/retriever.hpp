#pragma once

#include "types.hpp"
#include "document_encoder.hpp"
#include "mips.hpp"
#include <memory>
#include <vector>
#include <optional>

namespace realm {

/**
 * @brief Retrieval result
 */
struct RetrievalResult {
    DocumentId doc_id;
    Float relevance_score;
    std::optional<Document> document;  // Optional full document

    bool operator<(const RetrievalResult& other) const {
        return relevance_score > other.relevance_score;  // Higher scores first
    }
};

/**
 * @brief Document Retriever Component
 *
 * Responsible for retrieving relevant documents given a query.
 * Uses MIPS for efficient retrieval from large document collections.
 */
class Retriever {
public:
    /**
     * @brief Construct a retriever
     * @param encoder Document encoder
     * @param mips_retriever MIPS implementation
     * @param hyperparams Hyperparameters
     */
    Retriever(
        std::shared_ptr<IDocumentEncoder> encoder,
        std::unique_ptr<IMIPSRetriever> mips_retriever,
        const HyperParams& hyperparams
    );

    /**
     * @brief Index a collection of documents
     * @param documents Documents to index
     */
    void index_documents(const std::vector<Document>& documents);

    /**
     * @brief Retrieve top-k documents for a query
     * @param query Query to search for
     * @return Vector of top-k retrieval results
     */
    [[nodiscard]] std::vector<RetrievalResult>
        retrieve(const Query& query);

    /**
     * @brief Retrieve top-k documents for a query vector
     * @param query_vec Pre-encoded query vector
     * @return Vector of top-k retrieval results
     */
    [[nodiscard]] std::vector<RetrievalResult>
        retrieve(const DenseVector<Float>& query_vec);

    /**
     * @brief Get document vector by ID
     * @param doc_id Document ID
     * @return Document vector if found
     */
    [[nodiscard]] std::optional<DenseVector<Float>>
        get_document_vector(DocumentId doc_id) const;

    /**
     * @brief Get number of indexed documents
     * @return Number of documents
     */
    [[nodiscard]] size_t num_documents() const noexcept {
        return mips_retriever_->size();
    }

    /**
     * @brief Compute retrieval loss for training
     * @param query_vec Query vector
     * @param relevant_doc_ids IDs of relevant documents
     * @return Retrieval loss value
     */
    [[nodiscard]] Float compute_retrieval_loss(
        const DenseVector<Float>& query_vec,
        const std::vector<DocumentId>& relevant_doc_ids
    );

private:
    std::shared_ptr<IDocumentEncoder> encoder_;
    std::unique_ptr<IMIPSRetriever> mips_retriever_;
    HyperParams hyperparams_;

    // Cache of document vectors
    std::vector<std::pair<DocumentId, DenseVector<Float>>> document_cache_;
};

/**
 * @brief Retriever Trainer
 *
 * Handles training of the retriever component using MIPS optimization.
 */
class RetrieverTrainer {
public:
    explicit RetrieverTrainer(
        std::shared_ptr<Retriever> retriever,
        const HyperParams& hyperparams
    );

    /**
     * @brief Train the retriever
     * @param queries Training queries
     * @param relevance_labels Relevance labels (query_id -> {relevant_doc_ids})
     * @return Final training statistics
     */
    TrainingStats train(
        const std::vector<Query>& queries,
        const std::vector<std::vector<DocumentId>>& relevance_labels
    );

    /**
     * @brief Train for one epoch
     * @param queries Training queries
     * @param relevance_labels Relevance labels
     * @return Epoch training statistics
     */
    TrainingStats train_epoch(
        const std::vector<Query>& queries,
        const std::vector<std::vector<DocumentId>>& relevance_labels
    );

private:
    std::shared_ptr<Retriever> retriever_;
    HyperParams hyperparams_;

    Float compute_batch_loss(
        const std::vector<Query>& batch_queries,
        const std::vector<std::vector<DocumentId>>& batch_labels
    );
};

} // namespace realm
