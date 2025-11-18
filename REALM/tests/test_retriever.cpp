#include <gtest/gtest.h>
#include "realm/retriever.hpp"

using namespace realm;

class RetrieverTest : public ::testing::Test {
protected:
    void SetUp() override {
        params_.document_vector_size = 64;
        params_.top_k_documents = 3;

        encoder_ = std::make_shared<SimpleEncoder>(params_.document_vector_size);
        auto mips = std::make_unique<BruteForceMIPS>(params_.document_vector_size);

        retriever_ = std::make_shared<Retriever>(
            encoder_,
            std::move(mips),
            params_
        );

        // Create test documents
        documents_ = {
            {0, "Machine learning is a subset of artificial intelligence"},
            {1, "Deep learning uses neural networks"},
            {2, "Natural language processing enables text understanding"},
            {3, "Computer vision processes images and videos"},
            {4, "Reinforcement learning learns through trial and error"}
        };

        retriever_->index_documents(documents_);
    }

    HyperParams params_;
    std::shared_ptr<SimpleEncoder> encoder_;
    std::shared_ptr<Retriever> retriever_;
    std::vector<Document> documents_;
};

TEST_F(RetrieverTest, NumDocuments) {
    EXPECT_EQ(retriever_->num_documents(), 5);
}

TEST_F(RetrieverTest, RetrieveByQuery) {
    Query query(0, "What is machine learning?");
    auto results = retriever_->retrieve(query);

    EXPECT_EQ(results.size(), params_.top_k_documents);

    // Results should be sorted by relevance
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_GE(results[i-1].relevance_score, results[i].relevance_score);
    }
}

TEST_F(RetrieverTest, RetrieveByVector) {
    Query query(0, "neural networks deep learning");
    auto query_vec = encoder_->encode_query(query);

    auto results = retriever_->retrieve(query_vec);

    EXPECT_EQ(results.size(), params_.top_k_documents);
}

TEST_F(RetrieverTest, GetDocumentVector) {
    auto vec_opt = retriever_->get_document_vector(0);

    ASSERT_TRUE(vec_opt.has_value());
    EXPECT_EQ(vec_opt->dimension(), params_.document_vector_size);
}

TEST_F(RetrieverTest, GetNonexistentDocument) {
    auto vec_opt = retriever_->get_document_vector(999);
    EXPECT_FALSE(vec_opt.has_value());
}

TEST_F(RetrieverTest, ComputeRetrievalLoss) {
    Query query(0, "machine learning");
    auto query_vec = encoder_->encode_query(query);

    std::vector<DocumentId> relevant_docs = {0, 1};
    Float loss = retriever_->compute_retrieval_loss(query_vec, relevant_docs);

    EXPECT_GT(loss, 0.0f);
    EXPECT_LT(loss, 1000.0f);  // Sanity check
}

class RetrieverTrainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        params_.document_vector_size = 64;
        params_.top_k_documents = 2;
        params_.num_epochs = 2;
        params_.batch_size = 2;

        encoder_ = std::make_shared<SimpleEncoder>(params_.document_vector_size);
        auto mips = std::make_unique<BruteForceMIPS>(params_.document_vector_size);

        retriever_ = std::make_shared<Retriever>(
            encoder_,
            std::move(mips),
            params_
        );

        documents_ = {
            {0, "First document"},
            {1, "Second document"},
            {2, "Third document"}
        };

        retriever_->index_documents(documents_);

        trainer_ = std::make_unique<RetrieverTrainer>(retriever_, params_);
    }

    HyperParams params_;
    std::shared_ptr<SimpleEncoder> encoder_;
    std::shared_ptr<Retriever> retriever_;
    std::vector<Document> documents_;
    std::unique_ptr<RetrieverTrainer> trainer_;
};

TEST_F(RetrieverTrainerTest, TrainEpoch) {
    std::vector<Query> queries = {
        {0, "query one"},
        {1, "query two"}
    };

    std::vector<std::vector<DocumentId>> labels = {
        {0, 1},
        {1, 2}
    };

    auto stats = trainer_->train_epoch(queries, labels);

    EXPECT_GT(stats.retrieval_loss, 0.0f);
    EXPECT_EQ(stats.total_loss, stats.retrieval_loss);
}

TEST_F(RetrieverTrainerTest, Train) {
    std::vector<Query> queries = {
        {0, "first query"}
    };

    std::vector<std::vector<DocumentId>> labels = {
        {0}
    };

    auto stats = trainer_->train(queries, labels);

    EXPECT_GT(stats.retrieval_loss, 0.0f);
}
