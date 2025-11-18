#include "realm/rag_model.hpp"
#include "realm/document_encoder.hpp"
#include "realm/mips.hpp"
#include <iostream>
#include <memory>
#include <random>

using namespace realm;

int main() {
    std::cout << "=== RAG Training Example ===" << std::endl << std::endl;

    // Configure hyperparameters
    HyperParams params;
    params.document_vector_size = 128;
    params.query_vector_size = 128;
    params.retriever_hidden_size = 256;
    params.top_k_documents = 5;
    params.num_epochs = 10;
    params.batch_size = 16;
    params.learning_rate = 0.001f;

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Document Vector Size: " << params.document_vector_size << std::endl;
    std::cout << "  Retriever Hidden Size: " << params.retriever_hidden_size << std::endl;
    std::cout << "  Top-K Documents: " << params.top_k_documents << std::endl;
    std::cout << "  Epochs: " << params.num_epochs << std::endl;
    std::cout << "  Batch Size: " << params.batch_size << std::endl;
    std::cout << "  Learning Rate: " << params.learning_rate << std::endl << std::endl;

    // Create components
    auto encoder = std::make_shared<SimpleEncoder>(params.document_vector_size);
    auto mips = std::make_unique<BruteForceMIPS>(params.document_vector_size);
    auto retriever = std::make_shared<Retriever>(encoder, std::move(mips), params);
    auto generator = std::make_shared<Generator>(params.retriever_hidden_size, params);
    auto rag_model = std::make_shared<RAGModel>(retriever, generator, params);

    // Create training data
    std::cout << "Generating training data..." << std::endl;

    std::vector<Document> training_documents;
    for (size_t i = 0; i < 50; ++i) {
        std::string content = "Training document " + std::to_string(i) +
                            " contains important information about topic " +
                            std::to_string(i % 10);
        training_documents.emplace_back(i, content);
    }

    std::vector<Query> training_queries;
    std::vector<std::vector<DocumentId>> relevance_labels;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> doc_dist(0, static_cast<int>(training_documents.size() - 1));

    for (size_t i = 0; i < 20; ++i) {
        std::string query_content = "Query about topic " + std::to_string(i % 10);
        training_queries.emplace_back(i, query_content);

        // Generate random relevance labels
        std::vector<DocumentId> relevant_docs;
        for (size_t j = 0; j < 3; ++j) {
            relevant_docs.push_back(static_cast<DocumentId>(doc_dist(gen)));
        }
        relevance_labels.push_back(relevant_docs);
    }

    std::cout << "  Training Documents: " << training_documents.size() << std::endl;
    std::cout << "  Training Queries: " << training_queries.size() << std::endl << std::endl;

    // Configure training
    RAGTrainer::TrainingConfig training_config;
    training_config.joint_training = false;  // Train separately first
    training_config.retriever_pretrain_epochs = 5;
    training_config.generator_pretrain_epochs = 5;
    training_config.joint_training_epochs = 10;
    training_config.log_every_n_steps = 1;
    training_config.validate_every_n_epochs = 5;

    std::cout << "Training Configuration:" << std::endl;
    std::cout << "  Joint Training: " << (training_config.joint_training ? "Yes" : "No") << std::endl;
    std::cout << "  Retriever Pretrain Epochs: " << training_config.retriever_pretrain_epochs << std::endl;
    std::cout << "  Generator Pretrain Epochs: " << training_config.generator_pretrain_epochs << std::endl;
    std::cout << "  Joint Training Epochs: " << training_config.joint_training_epochs << std::endl << std::endl;

    // Create trainer and train
    RAGTrainer trainer(rag_model, params, training_config);

    std::cout << "Starting training..." << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    auto final_stats = trainer.train(training_documents, training_queries, relevance_labels);

    std::cout << std::string(70, '=') << std::endl;
    std::cout << "\nTraining Complete!" << std::endl;
    std::cout << "Final Statistics:" << std::endl;
    std::cout << "  MLM Loss: " << final_stats.mlm_loss << std::endl;
    std::cout << "  Retrieval Loss: " << final_stats.retrieval_loss << std::endl;
    std::cout << "  Total Loss: " << final_stats.total_loss << std::endl << std::endl;

    // Test the trained model
    std::cout << "Testing trained model..." << std::endl;
    Query test_query{100, "Tell me about topic 5"};

    auto [result, retrieved] = rag_model->answer_with_retrieval(test_query, 3);

    std::cout << "Query: " << test_query.content << std::endl;
    std::cout << "Retrieved Documents:" << std::endl;
    for (size_t i = 0; i < retrieved.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] Doc " << retrieved[i].doc_id
                  << " (score: " << retrieved[i].relevance_score << ")" << std::endl;
    }
    std::cout << "Generated Answer: " << result.generated_text << std::endl;

    std::cout << "\nExample completed successfully!" << std::endl;

    return 0;
}
