#include "realm/retriever.hpp"
#include "realm/document_encoder.hpp"
#include "realm/mips.hpp"
#include <iostream>
#include <iomanip>
#include <memory>

using namespace realm;

int main() {
    std::cout << "=== Document Retrieval Example ===" << std::endl << std::endl;

    // Configure hyperparameters
    HyperParams params;
    params.document_vector_size = 256;
    params.top_k_documents = 5;

    std::cout << "Creating encoder and retriever..." << std::endl;

    // Create document encoder
    auto encoder = std::make_shared<SimpleEncoder>(params.document_vector_size, 5000);

    // Compare different MIPS implementations
    std::cout << "\n--- Testing Brute Force MIPS ---" << std::endl;
    auto brute_force_mips = std::make_unique<BruteForceMIPS>(params.document_vector_size);
    auto brute_force_retriever = std::make_shared<Retriever>(
        encoder,
        std::move(brute_force_mips),
        params
    );

    // Create a larger document collection
    std::vector<Document> documents;
    documents.reserve(100);

    std::vector<std::string> topics = {
        "machine learning", "deep learning", "neural networks",
        "natural language processing", "computer vision",
        "reinforcement learning", "data science", "big data",
        "cloud computing", "distributed systems"
    };

    for (size_t i = 0; i < 100; ++i) {
        std::string topic = topics[i % topics.size()];
        std::string content = "This document discusses " + topic +
                            " and its applications in modern technology " +
                            std::to_string(i);
        documents.emplace_back(i, content);
    }

    std::cout << "Indexing " << documents.size() << " documents..." << std::endl;
    brute_force_retriever->index_documents(documents);
    std::cout << "Indexed " << brute_force_retriever->num_documents() << " documents" << std::endl;

    // Test queries
    std::vector<Query> queries = {
        {0, "What are neural networks and deep learning?"},
        {1, "How does natural language processing work?"},
        {2, "Explain distributed computing systems"}
    };

    for (const auto& query : queries) {
        std::cout << "\nQuery: " << query.content << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        auto results = brute_force_retriever->retrieve(query);

        std::cout << "Top " << results.size() << " Results:" << std::endl;
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << std::setw(2) << (i + 1) << ". "
                      << "Document " << std::setw(3) << results[i].doc_id
                      << " | Score: " << std::fixed << std::setprecision(4)
                      << results[i].relevance_score << std::endl;
        }
    }

    // Test Asymmetric LSH
    std::cout << "\n\n--- Testing Asymmetric LSH MIPS ---" << std::endl;

    AsymmetricLSH::Params lsh_params;
    lsh_params.num_hash_tables = 5;
    lsh_params.hash_size = 32;

    auto alsh_mips = std::make_unique<AsymmetricLSH>(params.document_vector_size, lsh_params);
    auto alsh_retriever = std::make_shared<Retriever>(
        encoder,
        std::move(alsh_mips),
        params
    );

    std::cout << "Indexing documents with ALSH..." << std::endl;
    alsh_retriever->index_documents(documents);
    std::cout << "ALSH indexing complete!" << std::endl;

    std::cout << "\nComparing retrieval results:" << std::endl;
    Query test_query{0, "machine learning and artificial intelligence"};

    auto bf_results = brute_force_retriever->retrieve(test_query);
    auto alsh_results = alsh_retriever->retrieve(test_query);

    std::cout << "\nBrute Force Results:" << std::endl;
    for (size_t i = 0; i < std::min(size_t(3), bf_results.size()); ++i) {
        std::cout << "  " << (i + 1) << ". Doc " << bf_results[i].doc_id
                  << " (score: " << bf_results[i].relevance_score << ")" << std::endl;
    }

    std::cout << "\nALSH Results:" << std::endl;
    for (size_t i = 0; i < std::min(size_t(3), alsh_results.size()); ++i) {
        std::cout << "  " << (i + 1) << ". Doc " << alsh_results[i].doc_id
                  << " (score: " << alsh_results[i].relevance_score << ")" << std::endl;
    }

    std::cout << "\nExample completed successfully!" << std::endl;

    return 0;
}
