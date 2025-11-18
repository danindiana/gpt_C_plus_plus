#include "mlc/mlc_network.hpp"
#include <gtest/gtest.h>

using namespace mlc;

TEST(MLCNetworkTest, Construction) {
    MLCConfig config;
    EXPECT_NO_THROW({
        MLCNetwork network(config);
    });
}

TEST(MLCNetworkTest, InvalidConfig) {
    MLCConfig invalid_config;
    invalid_config.hidden_size = 0;

    EXPECT_THROW({
        MLCNetwork network(invalid_config);
    }, std::invalid_argument);
}

TEST(MLCNetworkTest, TrainOnEpisode) {
    MLCConfig config;
    MLCNetwork network(config);

    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.add_study_example(Example("skip", {"square"}));
    ep.set_query_example(Example("jump", {"circle"}));

    EXPECT_NO_THROW({
        Float loss = network.train_on_episode(ep);
        EXPECT_GE(loss, 0.0);
    });
}

TEST(MLCNetworkTest, InvalidEpisode) {
    MLCConfig config;
    MLCNetwork network(config);

    Episode invalid_ep;  // Empty episode

    EXPECT_THROW({
        network.train_on_episode(invalid_ep);
    }, std::invalid_argument);
}

TEST(MLCNetworkTest, Evaluate) {
    MLCConfig config;
    MLCNetwork network(config);

    std::vector<Episode> episodes;
    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.set_query_example(Example("jump", {"circle"}));
    episodes.push_back(ep);

    Float accuracy = network.evaluate(episodes);
    EXPECT_GE(accuracy, 0.0);
    EXPECT_LE(accuracy, 1.0);
}

TEST(MLCNetworkTest, Predict) {
    MLCConfig config;
    MLCNetwork network(config);

    std::vector<Example> context = {
        Example("jump", {"circle"}),
        Example("skip", {"square"})
    };

    auto output = network.predict("jump", context);
    EXPECT_FALSE(output.empty());
}

TEST(MLCNetworkTest, StatsTracking) {
    MLCConfig config;
    MLCNetwork network(config);

    const auto& stats_before = network.stats();
    EXPECT_EQ(stats_before.total_examples, 0);

    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.set_query_example(Example("skip", {"square"}));

    network.train_on_episode(ep);

    const auto& stats_after = network.stats();
    EXPECT_EQ(stats_after.total_examples, 1);
}

TEST(MLCNetworkTest, ResetStats) {
    MLCConfig config;
    MLCNetwork network(config);

    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.set_query_example(Example("skip", {"square"}));

    network.train_on_episode(ep);
    EXPECT_GT(network.stats().total_examples, 0);

    network.reset_stats();
    EXPECT_EQ(network.stats().total_examples, 0);
}

TEST(MLCNetworkTest, MultipleEpisodes) {
    MLCConfig config;
    config.learning_rate = 0.01;
    MLCNetwork network(config);

    for (int i = 0; i < 10; ++i) {
        Episode ep;
        ep.add_study_example(Example("jump", {"circle"}));
        ep.set_query_example(Example("jump", {"circle"}));

        Float loss = network.train_on_episode(ep);
        EXPECT_GE(loss, 0.0);
    }

    EXPECT_EQ(network.stats().total_examples, 10);
}
