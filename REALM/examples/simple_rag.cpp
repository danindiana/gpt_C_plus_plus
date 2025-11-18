#include "realm/rag_model.hpp"
#include "realm/document_encoder.hpp"
#include "realm/mips.hpp"
#include <iostream>
#include <memory>

using namespace realm;

int main() {
    std::cout << "=== Simple RAG Example ===" << std::endl << std::endl;

    // Configure hyperparameters
    HyperParams params;
    params.document_vector_size = 128;
    params.query_vector_size = 128;
    params.top_k_documents = 3;

    // Create document encoder
    auto encoder = std::make_shared<SimpleEncoder>(params.document_vector_size);

    // Create MIPS retriever
    auto mips = std::make_unique<BruteForceMIPS>(params.document_vector_size);

    // Create retriever
    auto retriever = std::make_shared<Retriever>(
        encoder,
        std::move(mips),
        params
    );

    // Create generator
    auto generator = std::make_shared<Generator>(params.retriever_hidden_size, params);

    // Create RAG model
    auto rag_model = std::make_shared<RAGModel>(retriever, generator, params);

    // Create document collection
    std::vector<Document> documents = {
        {0, "The quick brown fox jumps over the lazy dog"},
        {1, "Machine learning is a subset of artificial intelligence"},
        {2, "Natural language processing enables computers to understand human language"},
        {3, "Retrieval augmented generation combines retrieval with text generation"},
        {4, "Deep learning uses neural networks with multiple layers"}
    };

    std::cout << "Indexing " << documents.size() << " documents..." << std::endl;
    rag_model->index_documents(documents);
    std::cout << "Indexing complete!" << std::endl << std::endl;

    // Test queries
    std::vector<Query> queries = {
        {0, "What is machine learning?"},
        {1, "Tell me about language processing"},
        {2, "Explain RAG models"}
    };

    for (const auto& query : queries) {
        std::cout << "Query: " << query.content << std::endl;

        auto [result, retrieved_docs] = rag_model->answer_with_retrieval(query, 3);

        std::cout << "Retrieved Documents:" << std::endl;
        for (size_t i = 0; i < retrieved_docs.size(); ++i) {
            std::cout << "  [" << (i + 1) << "] Doc " << retrieved_docs[i].doc_id
                      << " (score: " << retrieved_docs[i].relevance_score << ")" << std::endl;
        }

        std::cout << "Generated Answer: " << result.generated_text << std::endl;
        std::cout << "Confidence: " << result.confidence_score << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Example completed successfully!" << std::endl;

    return 0;
}
