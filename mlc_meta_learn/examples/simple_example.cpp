#include "mlc/mlc.hpp"
#include <iostream>

/**
 * @brief Simple example demonstrating basic MLC usage
 *
 * This example shows how to:
 * 1. Create examples and episodes manually
 * 2. Create and configure an MLC network
 * 3. Train on a few episodes
 * 4. Make predictions
 */

int main() {
    using namespace mlc;

    std::cout << "Simple MLC Example\n";
    std::cout << "==================\n\n";

    // Create some examples manually
    Example ex1("jump", {"circle"});
    Example ex2("skip", {"square"});
    Example ex3("run", {"triangle"});

    std::cout << "Created examples:\n";
    std::cout << "  " << ex1 << "\n";
    std::cout << "  " << ex2 << "\n";
    std::cout << "  " << ex3 << "\n\n";

    // Create an episode
    Episode episode;
    episode.add_study_example(ex1);
    episode.add_study_example(ex2);
    episode.set_query_example(Example("jump", {"circle"}));

    std::cout << "Created episode with " << episode.num_study_examples()
              << " study examples\n\n";

    // Create a simple network configuration
    MLCConfig config;
    config.hidden_size = 64;
    config.embedding_dim = 32;
    config.attention_heads = 2;
    config.learning_rate = 0.01;

    // Create the network
    std::cout << "Creating MLC network...\n";
    MLCNetwork network(config);

    // Train on the episode a few times
    std::cout << "Training on episode...\n";
    for (int i = 0; i < 10; ++i) {
        Float loss = network.train_on_episode(episode);
        std::cout << "  Iteration " << (i + 1) << ", Loss: " << loss << "\n";
    }

    // Make a prediction
    std::cout << "\nMaking predictions:\n";

    std::vector<Example> context = {ex1, ex2, ex3};

    auto pred1 = network.predict("jump", context);
    std::cout << "  Predict 'jump': [";
    for (size_t i = 0; i < pred1.size(); ++i) {
        std::cout << pred1[i];
        if (i < pred1.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";

    auto pred2 = network.predict("skip", context);
    std::cout << "  Predict 'skip': [";
    for (size_t i = 0; i < pred2.size(); ++i) {
        std::cout << pred2[i];
        if (i < pred2.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";

    std::cout << "\nExample completed!\n";
    return 0;
}
