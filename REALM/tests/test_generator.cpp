#include <gtest/gtest.h>
#include "realm/generator.hpp"

using namespace realm;

class GeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        params_.retriever_hidden_size = 128;
        generator_ = std::make_unique<Generator>(params_.retriever_hidden_size, params_);
    }

    HyperParams params_;
    std::unique_ptr<Generator> generator_;
};

TEST_F(GeneratorTest, Generate) {
    Query query(0, "What is machine learning?");

    std::vector<RetrievalResult> retrieved_docs;
    retrieved_docs.push_back({0, 0.9f});
    retrieved_docs.push_back({1, 0.7f});

    auto result = generator_->generate(query, retrieved_docs);

    EXPECT_FALSE(result.generated_text.empty());
    EXPECT_GT(result.confidence_score, 0.0f);
    EXPECT_LE(result.confidence_score, 1.0f);
    EXPECT_EQ(result.used_documents.size(), 2);
}

TEST_F(GeneratorTest, CreateMaskedTokens) {
    std::vector<std::string> tokens = {"This", "is", "a", "test", "sentence"};

    auto masked = generator_->create_masked_tokens(tokens, 0.5f);

    // With 50% probability, expect roughly half to be masked
    EXPECT_GT(masked.size(), 0);
    EXPECT_LE(masked.size(), tokens.size());

    for (const auto& mt : masked) {
        EXPECT_LT(mt.position, tokens.size());
        EXPECT_EQ(mt.masked_token, "[MASK]");
        EXPECT_FALSE(mt.original_token.empty());
    }
}

TEST_F(GeneratorTest, CreateMaskedTokensZeroProbability) {
    std::vector<std::string> tokens = {"This", "is", "a", "test"};

    auto masked = generator_->create_masked_tokens(tokens, 0.0f);

    EXPECT_EQ(masked.size(), 0);
}

TEST_F(GeneratorTest, PredictMaskedTokens) {
    std::vector<std::string> masked_input = {"This", "[MASK]", "a", "test"};
    std::vector<Document> context_docs = {
        {0, "This is context"}
    };

    auto predictions = generator_->predict_masked_tokens(masked_input, context_docs);

    EXPECT_EQ(predictions.size(), 1);  // One [MASK] token
    EXPECT_FALSE(predictions[0].empty());
}

TEST_F(GeneratorTest, ComputeMLMLoss) {
    std::vector<std::string> input = {"This", "is", "a", "test"};
    std::vector<size_t> masked_positions = {1, 3};
    std::vector<std::string> target_tokens = {"is", "test"};
    std::vector<Document> context_docs = {{0, "Context document"}};

    Float loss = generator_->compute_mlm_loss(
        input,
        masked_positions,
        target_tokens,
        context_docs
    );

    EXPECT_GT(loss, 0.0f);
}

class GeneratorTrainerTest : public ::testing::Test {
protected:
    void SetUp() override {
        params_.retriever_hidden_size = 64;
        params_.num_epochs = 2;
        params_.batch_size = 2;

        generator_ = std::make_shared<Generator>(params_.retriever_hidden_size, params_);
        trainer_ = std::make_unique<GeneratorTrainer>(generator_, params_);
    }

    HyperParams params_;
    std::shared_ptr<Generator> generator_;
    std::unique_ptr<GeneratorTrainer> trainer_;
};

TEST_F(GeneratorTrainerTest, TrainEpoch) {
    std::vector<Document> training_data = {
        {0, "First training document"},
        {1, "Second training document"}
    };

    std::vector<std::vector<Document>> context_docs = {
        {{0, "Context for first"}},
        {{0, "Context for second"}}
    };

    auto stats = trainer_->train_epoch(training_data, context_docs);

    EXPECT_GT(stats.mlm_loss, 0.0f);
    EXPECT_EQ(stats.total_loss, stats.mlm_loss);
}

TEST_F(GeneratorTrainerTest, Train) {
    std::vector<Document> training_data = {
        {0, "Training document"}
    };

    std::vector<std::vector<Document>> context_docs = {
        {{0, "Context"}}
    };

    auto stats = trainer_->train(training_data, context_docs);

    EXPECT_GT(stats.mlm_loss, 0.0f);
    EXPECT_GT(stats.epoch, 0);
}

TEST(MaskedTokenTest, Structure) {
    MaskedToken mt;
    mt.position = 5;
    mt.original_token = "hello";
    mt.masked_token = "[MASK]";

    EXPECT_EQ(mt.position, 5);
    EXPECT_EQ(mt.original_token, "hello");
    EXPECT_EQ(mt.masked_token, "[MASK]");
}
