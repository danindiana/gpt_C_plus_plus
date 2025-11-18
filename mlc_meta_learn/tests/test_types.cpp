#include "mlc/types.hpp"
#include <gtest/gtest.h>

using namespace mlc;

TEST(TypesTest, MLCConfigDefaults) {
    MLCConfig config;

    EXPECT_EQ(config.hidden_size, 128);
    EXPECT_EQ(config.num_layers, 2);
    EXPECT_EQ(config.embedding_dim, 64);
    EXPECT_EQ(config.attention_heads, 4);
    EXPECT_DOUBLE_EQ(config.learning_rate, 0.001);
    EXPECT_DOUBLE_EQ(config.dropout_rate, 0.1);
    EXPECT_EQ(config.batch_size, 32);
    EXPECT_TRUE(config.use_attention);
}

TEST(TypesTest, MLCConfigValidation) {
    MLCConfig valid_config;
    EXPECT_TRUE(valid_config.is_valid());

    MLCConfig invalid_config;
    invalid_config.hidden_size = 0;
    EXPECT_FALSE(invalid_config.is_valid());

    invalid_config = MLCConfig{};
    invalid_config.learning_rate = -0.1;
    EXPECT_FALSE(invalid_config.is_valid());

    invalid_config = MLCConfig{};
    invalid_config.dropout_rate = 1.5;
    EXPECT_FALSE(invalid_config.is_valid());
}

TEST(TypesTest, TrainingStatsInitialization) {
    TrainingStats stats;

    EXPECT_EQ(stats.epoch, 0);
    EXPECT_DOUBLE_EQ(stats.loss, 0.0);
    EXPECT_DOUBLE_EQ(stats.accuracy, 0.0);
    EXPECT_EQ(stats.total_examples, 0);
    EXPECT_EQ(stats.correct_predictions, 0);
}

TEST(TypesTest, TrainingStatsAccuracyUpdate) {
    TrainingStats stats;

    stats.total_examples = 100;
    stats.correct_predictions = 75;
    stats.update_accuracy();

    EXPECT_DOUBLE_EQ(stats.accuracy, 0.75);
}

TEST(TypesTest, TrainingStatsReset) {
    TrainingStats stats;
    stats.epoch = 10;
    stats.loss = 1.5;
    stats.accuracy = 0.8;
    stats.total_examples = 100;
    stats.correct_predictions = 80;

    stats.reset();

    EXPECT_EQ(stats.epoch, 0);
    EXPECT_DOUBLE_EQ(stats.loss, 0.0);
    EXPECT_DOUBLE_EQ(stats.accuracy, 0.0);
    EXPECT_EQ(stats.total_examples, 0);
    EXPECT_EQ(stats.correct_predictions, 0);
}

TEST(TypesTest, VersionInfo) {
    EXPECT_STREQ(VERSION, "1.0.0");
    EXPECT_EQ(VERSION_MAJOR, 1);
    EXPECT_EQ(VERSION_MINOR, 0);
    EXPECT_EQ(VERSION_PATCH, 0);
}
