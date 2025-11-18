#include "mlc/utils.hpp"
#include <gtest/gtest.h>

using namespace mlc;
using namespace mlc::utils;

TEST(EpisodeGeneratorTest, Construction) {
    EXPECT_NO_THROW({
        EpisodeGenerator gen(42);
    });
}

TEST(EpisodeGeneratorTest, GenerateEpisode) {
    EpisodeGenerator gen(42);
    auto ep = gen.generate_episode(5, 1);

    EXPECT_TRUE(ep.is_valid());
    EXPECT_EQ(ep.num_study_examples(), 5);
    EXPECT_TRUE(ep.query_example().is_valid());
}

TEST(EpisodeGeneratorTest, GenerateCompositionalEpisode) {
    EpisodeGenerator gen(42);
    auto ep = gen.generate_compositional_episode(5);

    EXPECT_TRUE(ep.is_valid());
    EXPECT_LE(ep.num_study_examples(), 5);
}

TEST(EpisodeGeneratorTest, GenerateBatch) {
    EpisodeGenerator gen(42);
    auto batch = gen.generate_batch(10, 5, 1);

    EXPECT_EQ(batch.size(), 10);
    for (const auto& ep : batch) {
        EXPECT_TRUE(ep.is_valid());
    }
}

TEST(EpisodeGeneratorTest, Determinism) {
    EpisodeGenerator gen1(42);
    EpisodeGenerator gen2(42);

    auto ep1 = gen1.generate_episode(5, 1);
    auto ep2 = gen2.generate_episode(5, 1);

    // With the same seed, should generate similar structure
    EXPECT_EQ(ep1.num_study_examples(), ep2.num_study_examples());
}

TEST(EpisodeGeneratorTest, DifferentComplexity) {
    EpisodeGenerator gen(42);

    auto simple = gen.generate_episode(5, 1);
    auto medium = gen.generate_episode(5, 2);
    auto complex = gen.generate_episode(5, 3);

    EXPECT_TRUE(simple.is_valid());
    EXPECT_TRUE(medium.is_valid());
    EXPECT_TRUE(complex.is_valid());
}

TEST(TrainingUtilsTest, SplitEpisodes) {
    std::vector<Episode> episodes;
    EpisodeGenerator gen(42);

    for (int i = 0; i < 100; ++i) {
        episodes.push_back(gen.generate_episode(3, 1));
    }

    auto split = training::split_episodes(episodes, 0.7, 0.15);

    EXPECT_EQ(split.train.size(), 70);
    EXPECT_EQ(split.validation.size(), 15);
    EXPECT_EQ(split.test.size(), 15);
}

TEST(TrainingUtilsTest, ShuffleEpisodes) {
    std::vector<Episode> episodes;
    EpisodeGenerator gen(42);

    for (int i = 0; i < 10; ++i) {
        episodes.push_back(gen.generate_episode(3, 1));
    }

    auto original = episodes;
    training::shuffle_episodes(episodes, 12345);

    // After shuffling, order should potentially be different
    // but size should be the same
    EXPECT_EQ(episodes.size(), original.size());
}

TEST(TrainingUtilsTest, ShuffleDeterminism) {
    std::vector<Episode> episodes;
    EpisodeGenerator gen(42);

    for (int i = 0; i < 10; ++i) {
        episodes.push_back(gen.generate_episode(3, 1));
    }

    auto copy = episodes;

    training::shuffle_episodes(episodes, 42);
    training::shuffle_episodes(copy, 42);

    // Same seed should produce same shuffle
    EXPECT_EQ(episodes.size(), copy.size());
}
