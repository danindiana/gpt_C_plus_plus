#include <gtest/gtest.h>
#include "realm/rag_model.hpp"

using namespace realm;

class RAGModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        params_.document_vector_size = 64;
        params_.query_vector_size = 64;
        params_.retriever_hidden_size = 128;
        params_.top_k_documents = 3;

        auto encoder = std::make_shared<SimpleEncoder>(params_.document_vector_size);
        auto mips = std::make_unique<BruteForceMIPS>(params_.document_vector_size);
        auto retriever = std::make_shared<Retriever>(
            encoder,
            std::move(mips),
            params_
        );
        auto generator = std::make_shared<Generator>(params_.retriever_hidden_size, params_);

        rag_model_ = std::make_shared<RAGModel>(retriever, generator, params_);

        documents_ = {
            {0, "Machine learning is powerful"},
            {1, "Deep learning uses neural networks"},
            {2, "AI systems can learn from data"}
        };

        rag_model_->index_documents(documents_);
    }

    HyperParams params_;
    std::shared_ptr<RAGModel> rag_model_;
    std::vector<Document> documents_;
};

TEST_F(RAGModelTest, AnswerQuery) {
    Query query(0, "What is machine learning?");

    auto result = rag_model_->answer_query(query);

    EXPECT_FALSE(result.generated_text.empty());
    EXPECT_GT(result.confidence_score, 0.0f);
    EXPECT_GT(result.used_documents.size(), 0);
}

TEST_F(RAGModelTest, AnswerWithRetrieval) {
    Query query(0, "Explain deep learning");

    auto [result, retrieved] = rag_model_->answer_with_retrieval(query, 2);

    EXPECT_FALSE(result.generated_text.empty());
    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(result.used_documents.size(), 2);
}

TEST_F(RAGModelTest, GetComponents) {
    auto retriever = rag_model_->get_retriever();
    auto generator = rag_model_->get_generator();

    EXPECT_NE(retriever, nullptr);
    EXPECT_NE(generator, nullptr);
}

TEST_F(RAGModelTest, IndexDocuments) {
    std::vector<Document> new_docs = {
        {10, "Additional document"}
    };

    // Re-indexing should work
    EXPECT_NO_THROW(rag_model_->index_documents(new_docs));
}

class RAGTrainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        params_.document_vector_size = 64;
        params_.retriever_hidden_size = 128;
        params_.num_epochs = 2;
        params_.batch_size = 2;

        auto encoder = std::make_shared<SimpleEncoder>(params_.document_vector_size);
        auto mips = std::make_unique<BruteForceMIPS>(params_.document_vector_size);
        auto retriever = std::make_shared<Retriever>(
            encoder,
            std::move(mips),
            params_
        );
        auto generator = std::make_shared<Generator>(params_.retriever_hidden_size, params_);

        rag_model_ = std::make_shared<RAGModel>(retriever, generator, params_);

        training_config_.joint_training = false;
        training_config_.retriever_pretrain_epochs = 1;
        training_config_.generator_pretrain_epochs = 1;
        training_config_.joint_training_epochs = 0;

        trainer_ = std::make_unique<RAGTrainer>(
            rag_model_,
            params_,
            training_config_
        );
    }

    HyperParams params_;
    RAGTrainer::TrainingConfig training_config_;
    std::shared_ptr<RAGModel> rag_model_;
    std::unique_ptr<RAGTrainer> trainer_;
};

TEST_F(RAGTrainerTest, PretrainRetriever) {
    std::vector<Document> documents = {
        {0, "Document one"},
        {1, "Document two"}
    };

    std::vector<Query> queries = {
        {0, "Query one"}
    };

    std::vector<std::vector<DocumentId>> labels = {
        {0}
    };

    auto stats = trainer_->pretrain_retriever(documents, queries, labels);

    EXPECT_GT(stats.retrieval_loss, 0.0f);
}

TEST_F(RAGTrainerTest, PretrainGenerator) {
    std::vector<Document> documents = {
        {0, "Training document"}
    };

    auto stats = trainer_->pretrain_generator(documents);

    EXPECT_GT(stats.mlm_loss, 0.0f);
}

TEST_F(RAGTrainerTest, Train) {
    std::vector<Document> documents = {
        {0, "First document"},
        {1, "Second document"}
    };

    std::vector<Query> queries = {
        {0, "Query one"}
    };

    std::vector<std::vector<DocumentId>> labels = {
        {0}
    };

    auto stats = trainer_->train(documents, queries, labels);

    EXPECT_GT(stats.total_loss, 0.0f);
}

TEST_F(RAGTrainerTest, JointTraining) {
    training_config_.joint_training = true;
    training_config_.joint_training_epochs = 2;

    trainer_ = std::make_unique<RAGTrainer>(
        rag_model_,
        params_,
        training_config_
    );

    std::vector<Document> documents = {{0, "Document"}};
    std::vector<Query> queries = {{0, "Query"}};
    std::vector<std::vector<DocumentId>> labels = {{0}};

    auto stats = trainer_->joint_train(documents, queries, labels);

    EXPECT_GT(stats.total_loss, 0.0f);
    EXPECT_GT(stats.retrieval_loss, 0.0f);
    EXPECT_GT(stats.mlm_loss, 0.0f);
}

TEST(TrainingConfigTest, DefaultValues) {
    RAGTrainer::TrainingConfig config;

    EXPECT_FALSE(config.joint_training);
    EXPECT_EQ(config.retriever_pretrain_epochs, 10);
    EXPECT_EQ(config.generator_pretrain_epochs, 10);
    EXPECT_EQ(config.joint_training_epochs, 50);
}

TEST(GenerationResultTest, Structure) {
    GenerationResult result;
    result.generated_text = "Generated answer";
    result.confidence_score = 0.95f;
    result.used_documents = {0, 1, 2};

    EXPECT_EQ(result.generated_text, "Generated answer");
    EXPECT_FLOAT_EQ(result.confidence_score, 0.95f);
    EXPECT_EQ(result.used_documents.size(), 3);
}
