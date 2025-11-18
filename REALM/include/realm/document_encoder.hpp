#pragma once

#include "types.hpp"
#include <memory>
#include <vector>
#include <optional>

namespace realm {

/**
 * @brief Interface for document encoding
 *
 * Converts documents and queries into dense vector representations
 * suitable for retrieval and generation tasks.
 */
class IDocumentEncoder {
public:
    virtual ~IDocumentEncoder() = default;

    /**
     * @brief Encode a document into a dense vector
     * @param doc Document to encode
     * @return Dense vector representation
     */
    [[nodiscard]] virtual DenseVector<Float> encode_document(const Document& doc) = 0;

    /**
     * @brief Encode a query into a dense vector
     * @param query Query to encode
     * @return Dense vector representation
     */
    [[nodiscard]] virtual DenseVector<Float> encode_query(const Query& query) = 0;

    /**
     * @brief Batch encode multiple documents
     * @param docs Documents to encode
     * @return Vector of dense representations
     */
    [[nodiscard]] virtual std::vector<DenseVector<Float>>
        encode_documents_batch(const std::vector<Document>& docs);

    /**
     * @brief Get the dimension of the encoded vectors
     * @return Vector dimension
     */
    [[nodiscard]] virtual VectorDim embedding_dimension() const noexcept = 0;
};

/**
 * @brief Simple bag-of-words encoder implementation
 *
 * A basic implementation using simple embeddings for demonstration.
 * In production, this would use BERT or similar transformer models.
 */
class SimpleEncoder : public IDocumentEncoder {
public:
    explicit SimpleEncoder(VectorDim dim, size_t vocab_size = 10000);

    [[nodiscard]] DenseVector<Float> encode_document(const Document& doc) override;
    [[nodiscard]] DenseVector<Float> encode_query(const Query& query) override;
    [[nodiscard]] VectorDim embedding_dimension() const noexcept override { return dim_; }

private:
    VectorDim dim_;
    size_t vocab_size_;
    std::vector<DenseVector<Float>> embeddings_;

    [[nodiscard]] DenseVector<Float> encode_text(const std::vector<std::string>& tokens);
    [[nodiscard]] size_t token_to_id(const std::string& token) const;
};

/**
 * @brief BERT-based encoder (placeholder for actual implementation)
 *
 * This would integrate with a BERT model for production use.
 */
class BERTEncoder : public IDocumentEncoder {
public:
    explicit BERTEncoder(const std::string& model_path, VectorDim dim = 768);

    [[nodiscard]] DenseVector<Float> encode_document(const Document& doc) override;
    [[nodiscard]] DenseVector<Float> encode_query(const Query& query) override;
    [[nodiscard]] VectorDim embedding_dimension() const noexcept override { return dim_; }

private:
    std::string model_path_;
    VectorDim dim_;
    // In production: std::unique_ptr<BERTModel> model_;
};

} // namespace realm
