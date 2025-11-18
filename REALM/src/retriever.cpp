#include "realm/retriever.hpp"
#include <algorithm>
#include <cmath>

namespace realm {

// Retriever implementation
Retriever::Retriever(
    std::shared_ptr<IDocumentEncoder> encoder,
    std::unique_ptr<IMIPSRetriever> mips_retriever,
    const HyperParams& hyperparams
) : encoder_(std::move(encoder)),
    mips_retriever_(std::move(mips_retriever)),
    hyperparams_(hyperparams) {}

void Retriever::index_documents(const std::vector<Document>& documents) {
    document_cache_.clear();
    document_cache_.reserve(documents.size());

    for (const auto& doc : documents) {
        DenseVector<Float> doc_vec = encoder_->encode_document(doc);
        mips_retriever_->add_document(doc.id, doc_vec);
        document_cache_.emplace_back(doc.id, std::move(doc_vec));
    }

    mips_retriever_->build_index();
}

std::vector<RetrievalResult>
Retriever::retrieve(const Query& query) {
    DenseVector<Float> query_vec = encoder_->encode_query(query);
    return retrieve(query_vec);
}

std::vector<RetrievalResult>
Retriever::retrieve(const DenseVector<Float>& query_vec) {
    auto mips_results = mips_retriever_->search(query_vec, hyperparams_.top_k_documents);

    std::vector<RetrievalResult> results;
    results.reserve(mips_results.size());

    for (const auto& mips_result : mips_results) {
        RetrievalResult result;
        result.doc_id = mips_result.doc_id;
        result.relevance_score = mips_result.score;
        results.push_back(result);
    }

    return results;
}

std::optional<DenseVector<Float>>
Retriever::get_document_vector(DocumentId doc_id) const {
    for (const auto& [id, vec] : document_cache_) {
        if (id == doc_id) {
            return vec;
        }
    }
    return std::nullopt;
}

Float Retriever::compute_retrieval_loss(
    const DenseVector<Float>& query_vec,
    const std::vector<DocumentId>& relevant_doc_ids
) {
    // Compute contrastive loss
    Float loss = 0.0f;

    // Get all document scores
    auto all_results = mips_retriever_->search(query_vec, document_cache_.size());

    // Compute softmax over all documents
    std::vector<Float> scores;
    scores.reserve(all_results.size());
    Float max_score = -std::numeric_limits<Float>::infinity();

    for (const auto& result : all_results) {
        scores.push_back(result.score);
        max_score = std::max(max_score, result.score);
    }

    // Numerical stability: subtract max score
    Float sum_exp = 0.0f;
    for (auto& score : scores) {
        score -= max_score;
        sum_exp += std::exp(score);
    }

    // Compute negative log likelihood for relevant documents
    for (DocumentId relevant_id : relevant_doc_ids) {
        // Find score for this document
        for (size_t i = 0; i < all_results.size(); ++i) {
            if (all_results[i].doc_id == relevant_id) {
                Float prob = std::exp(scores[i]) / sum_exp;
                loss -= std::log(prob + 1e-10f);  // Add small epsilon for stability
                break;
            }
        }
    }

    loss /= static_cast<Float>(relevant_doc_ids.size());
    return loss;
}

// RetrieverTrainer implementation
RetrieverTrainer::RetrieverTrainer(
    std::shared_ptr<Retriever> retriever,
    const HyperParams& hyperparams
) : retriever_(std::move(retriever)),
    hyperparams_(hyperparams) {}

TrainingStats RetrieverTrainer::train(
    const std::vector<Query>& queries,
    const std::vector<std::vector<DocumentId>>& relevance_labels
) {
    TrainingStats final_stats;

    for (size_t epoch = 0; epoch < hyperparams_.num_epochs; ++epoch) {
        TrainingStats epoch_stats = train_epoch(queries, relevance_labels);
        final_stats = epoch_stats;
    }

    return final_stats;
}

TrainingStats RetrieverTrainer::train_epoch(
    const std::vector<Query>& queries,
    const std::vector<std::vector<DocumentId>>& relevance_labels
) {
    TrainingStats stats;
    size_t num_batches = (queries.size() + hyperparams_.batch_size - 1) / hyperparams_.batch_size;

    for (size_t batch_idx = 0; batch_idx < num_batches; ++batch_idx) {
        size_t start_idx = batch_idx * hyperparams_.batch_size;
        size_t end_idx = std::min(start_idx + hyperparams_.batch_size, queries.size());

        std::vector<Query> batch_queries(
            queries.begin() + static_cast<ptrdiff_t>(start_idx),
            queries.begin() + static_cast<ptrdiff_t>(end_idx)
        );

        std::vector<std::vector<DocumentId>> batch_labels(
            relevance_labels.begin() + static_cast<ptrdiff_t>(start_idx),
            relevance_labels.begin() + static_cast<ptrdiff_t>(end_idx)
        );

        Float batch_loss = compute_batch_loss(batch_queries, batch_labels);
        stats.retrieval_loss += batch_loss;
    }

    stats.retrieval_loss /= static_cast<Float>(num_batches);
    stats.total_loss = stats.retrieval_loss;

    return stats;
}

Float RetrieverTrainer::compute_batch_loss(
    const std::vector<Query>& batch_queries,
    const std::vector<std::vector<DocumentId>>& batch_labels
) {
    Float total_loss = 0.0f;

    for (size_t i = 0; i < batch_queries.size(); ++i) {
        const auto& query = batch_queries[i];
        const auto& labels = batch_labels[i];

        // Encode query
        auto query_results = retriever_->retrieve(query);

        // Compute loss (simplified)
        // In production: backpropagate and update parameters
        Float loss = 0.0f;
        for (const auto& result : query_results) {
            bool is_relevant = std::find(
                labels.begin(), labels.end(), result.doc_id
            ) != labels.end();

            if (is_relevant) {
                loss -= std::log(result.relevance_score + 1e-10f);
            }
        }

        total_loss += loss;
    }

    return total_loss / static_cast<Float>(batch_queries.size());
}

} // namespace realm
