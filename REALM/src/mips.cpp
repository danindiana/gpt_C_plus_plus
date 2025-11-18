#include "realm/mips.hpp"
#include <algorithm>
#include <random>
#include <cmath>

namespace realm {

// BruteForceMIPS implementation
BruteForceMIPS::BruteForceMIPS(VectorDim dim)
    : dim_(dim) {}

void BruteForceMIPS::add_document(DocumentId doc_id, const DenseVector<Float>& vec) {
    if (vec.dimension() != dim_) {
        throw std::invalid_argument("Document vector dimension mismatch");
    }
    documents_.emplace_back(doc_id, vec);
}

void BruteForceMIPS::build_index() {
    // Brute force doesn't need index building
}

std::vector<MIPSResult>
BruteForceMIPS::search(const DenseVector<Float>& query_vec, size_t k) {
    if (query_vec.dimension() != dim_) {
        throw std::invalid_argument("Query vector dimension mismatch");
    }

    std::vector<MIPSResult> results;
    results.reserve(documents_.size());

    // Compute inner product with all documents
    for (const auto& [doc_id, doc_vec] : documents_) {
        Float score = query_vec.dot(doc_vec);
        results.push_back({doc_id, score});
    }

    // Sort by score (descending) and take top-k
    size_t top_k = std::min(k, results.size());
    std::partial_sort(
        results.begin(),
        results.begin() + static_cast<ptrdiff_t>(top_k),
        results.end(),
        [](const MIPSResult& a, const MIPSResult& b) {
            return a.score > b.score;
        }
    );

    results.resize(top_k);
    return results;
}

// AsymmetricLSH implementation
AsymmetricLSH::AsymmetricLSH(VectorDim dim, const Params& params)
    : dim_(dim), params_(params) {
    hash_tables_.resize(params.num_hash_tables);
}

void AsymmetricLSH::add_document(DocumentId doc_id, const DenseVector<Float>& vec) {
    if (vec.dimension() != dim_) {
        throw std::invalid_argument("Document vector dimension mismatch");
    }
    documents_.emplace_back(doc_id, vec);
    index_built_ = false;
}

void AsymmetricLSH::build_index() {
    // Build hash tables
    for (size_t table_idx = 0; table_idx < params_.num_hash_tables; ++table_idx) {
        hash_tables_[table_idx].clear();
        hash_tables_[table_idx].reserve(documents_.size());

        for (size_t doc_idx = 0; doc_idx < documents_.size(); ++doc_idx) {
            const auto& [doc_id, doc_vec] = documents_[doc_idx];
            DenseVector<Float> transformed = transform_document(doc_vec);
            [[maybe_unused]] size_t hash_val = hash_vector(transformed, table_idx);

            // Store document index in hash table
            // In production: use hash_val to bucket documents
            hash_tables_[table_idx].push_back(doc_idx);
        }
    }

    index_built_ = true;
}

std::vector<MIPSResult>
AsymmetricLSH::search(const DenseVector<Float>& query_vec, size_t k) {
    if (query_vec.dimension() != dim_) {
        throw std::invalid_argument("Query vector dimension mismatch");
    }

    if (!index_built_) {
        throw std::runtime_error("Index not built. Call build_index() first.");
    }

    // Transform query
    DenseVector<Float> transformed_query = transform_query(query_vec);

    // Collect candidate documents from hash tables
    std::vector<size_t> candidates;
    for (size_t table_idx = 0; table_idx < params_.num_hash_tables; ++table_idx) {
        [[maybe_unused]] size_t hash_val = hash_vector(transformed_query, table_idx);
        // In a real implementation, we'd look up the bucket using hash_val
        // For now, just add all documents from the table
        for (size_t doc_idx : hash_tables_[table_idx]) {
            candidates.push_back(doc_idx);
        }
    }

    // Remove duplicates
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(
        std::unique(candidates.begin(), candidates.end()),
        candidates.end()
    );

    // Compute exact scores for candidates
    std::vector<MIPSResult> results;
    results.reserve(candidates.size());

    for (size_t doc_idx : candidates) {
        const auto& [doc_id, doc_vec] = documents_[doc_idx];
        Float score = query_vec.dot(doc_vec);
        results.push_back({doc_id, score});
    }

    // Sort and return top-k
    size_t top_k = std::min(k, results.size());
    std::partial_sort(
        results.begin(),
        results.begin() + static_cast<ptrdiff_t>(top_k),
        results.end(),
        [](const MIPSResult& a, const MIPSResult& b) {
            return a.score > b.score;
        }
    );

    results.resize(top_k);
    return results;
}

DenseVector<Float> AsymmetricLSH::transform_query(const DenseVector<Float>& query) const {
    // Asymmetric transformation for query: P(q) = [q; ||q||^2/2; ||q||^4/4; ...]
    VectorDim transformed_dim = dim_ + 2;  // Add two extra dimensions
    DenseVector<Float> result(transformed_dim);

    Float norm_sq = 0.0f;
    for (size_t i = 0; i < dim_; ++i) {
        result[i] = query[i];
        norm_sq += query[i] * query[i];
    }

    result[dim_] = norm_sq / 2.0f;
    result[dim_ + 1] = (norm_sq * norm_sq) / 4.0f;

    return result;
}

DenseVector<Float> AsymmetricLSH::transform_document(const DenseVector<Float>& doc) const {
    // Asymmetric transformation for document: Q(x) = [x; 1; 1; ...]
    VectorDim transformed_dim = dim_ + 2;
    DenseVector<Float> result(transformed_dim);

    for (size_t i = 0; i < dim_; ++i) {
        result[i] = doc[i];
    }

    result[dim_] = 1.0f;
    result[dim_ + 1] = 1.0f;

    return result;
}

size_t AsymmetricLSH::hash_vector(const DenseVector<Float>& vec, size_t table_idx) const {
    // Simple hash function using random projections
    std::mt19937 gen(static_cast<unsigned>(table_idx));  // Use table index as seed
    std::normal_distribution<Float> dist(0.0f, 1.0f);

    Float projection = 0.0f;
    for (size_t i = 0; i < vec.dimension(); ++i) {
        projection += vec[i] * dist(gen);
    }

    // Quantize to bucket
    size_t bucket = static_cast<size_t>(std::floor(projection / params_.r));
    return bucket % params_.hash_size;
}

// FAISSRetriever implementation (placeholder)
FAISSRetriever::FAISSRetriever(VectorDim dim, const std::string& index_type)
    : dim_(dim), index_type_(index_type) {
    // In production: initialize FAISS index
    (void)index_type;  // Suppress unused parameter warning in placeholder
}

void FAISSRetriever::add_document(DocumentId doc_id, const DenseVector<Float>& vec) {
    // In production: add to FAISS index
    (void)doc_id;  // Suppress unused parameter warning in placeholder
    (void)vec;     // Suppress unused parameter warning in placeholder
    ++num_docs_;
}

void FAISSRetriever::build_index() {
    // In production: train and build FAISS index
}

std::vector<MIPSResult>
FAISSRetriever::search(const DenseVector<Float>& query_vec, size_t k) {
    // Placeholder implementation
    // In production: use FAISS index to search
    (void)query_vec;  // Suppress unused parameter warning in placeholder
    (void)k;          // Suppress unused parameter warning in placeholder
    return {};
}

} // namespace realm
