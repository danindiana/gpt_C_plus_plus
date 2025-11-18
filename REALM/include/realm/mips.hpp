#pragma once

#include "types.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <utility>

namespace realm {

/**
 * @brief Maximum Inner Product Search (MIPS) result
 */
struct MIPSResult {
    DocumentId doc_id;
    Float score;

    bool operator<(const MIPSResult& other) const {
        return score < other.score;
    }
};

/**
 * @brief Interface for MIPS retrieval
 *
 * Implements efficient maximum inner product search for document retrieval.
 */
class IMIPSRetriever {
public:
    virtual ~IMIPSRetriever() = default;

    /**
     * @brief Add a document vector to the index
     * @param doc_id Document identifier
     * @param vec Document vector
     */
    virtual void add_document(DocumentId doc_id, const DenseVector<Float>& vec) = 0;

    /**
     * @brief Build the index after adding all documents
     */
    virtual void build_index() = 0;

    /**
     * @brief Search for top-k documents with highest inner product
     * @param query_vec Query vector
     * @param k Number of results to return
     * @return Vector of top-k results
     */
    [[nodiscard]] virtual std::vector<MIPSResult>
        search(const DenseVector<Float>& query_vec, size_t k) = 0;

    /**
     * @brief Get total number of indexed documents
     * @return Number of documents
     */
    [[nodiscard]] virtual size_t size() const noexcept = 0;
};

/**
 * @brief Brute-force MIPS implementation
 *
 * Simple but accurate implementation for baseline and small datasets.
 */
class BruteForceMIPS : public IMIPSRetriever {
public:
    explicit BruteForceMIPS(VectorDim dim);

    void add_document(DocumentId doc_id, const DenseVector<Float>& vec) override;
    void build_index() override;
    [[nodiscard]] std::vector<MIPSResult>
        search(const DenseVector<Float>& query_vec, size_t k) override;
    [[nodiscard]] size_t size() const noexcept override { return documents_.size(); }

private:
    VectorDim dim_;
    std::vector<std::pair<DocumentId, DenseVector<Float>>> documents_;
};

/**
 * @brief Asymmetric LSH for MIPS
 *
 * Efficient approximate MIPS using Asymmetric Locality Sensitive Hashing.
 */
class AsymmetricLSH : public IMIPSRetriever {
public:
    struct Params {
        size_t num_hash_tables{10};
        size_t hash_size{64};
        Float U{1.0f};  // Upper bound on vector norms
        Float r{0.5f};  // Bucket width
    };

    AsymmetricLSH(VectorDim dim, const Params& params);

    void add_document(DocumentId doc_id, const DenseVector<Float>& vec) override;
    void build_index() override;
    [[nodiscard]] std::vector<MIPSResult>
        search(const DenseVector<Float>& query_vec, size_t k) override;
    [[nodiscard]] size_t size() const noexcept override { return documents_.size(); }

private:
    VectorDim dim_;
    Params params_;
    std::vector<std::pair<DocumentId, DenseVector<Float>>> documents_;
    std::vector<std::vector<size_t>> hash_tables_;
    bool index_built_{false};

    [[nodiscard]] DenseVector<Float> transform_query(const DenseVector<Float>& query) const;
    [[nodiscard]] DenseVector<Float> transform_document(const DenseVector<Float>& doc) const;
    [[nodiscard]] size_t hash_vector(const DenseVector<Float>& vec, size_t table_idx) const;
};

/**
 * @brief FAISS-based MIPS (for production use)
 *
 * Wrapper around FAISS library for high-performance retrieval.
 * This would require linking against FAISS in production.
 */
class FAISSRetriever : public IMIPSRetriever {
public:
    explicit FAISSRetriever(VectorDim dim, const std::string& index_type = "Flat");

    void add_document(DocumentId doc_id, const DenseVector<Float>& vec) override;
    void build_index() override;
    [[nodiscard]] std::vector<MIPSResult>
        search(const DenseVector<Float>& query_vec, size_t k) override;
    [[nodiscard]] size_t size() const noexcept override { return num_docs_; }

private:
    VectorDim dim_;
    std::string index_type_;
    size_t num_docs_{0};
    // In production: std::unique_ptr<faiss::Index> index_;
};

} // namespace realm
