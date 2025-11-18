#include "mlc/mlc.hpp"
#include <iostream>
#include <iomanip>

/**
 * @brief Test compositional generalization
 *
 * This example demonstrates the key capability of MLC:
 * learning to compose known primitives in novel ways
 */

int main() {
    using namespace mlc;

    std::cout << "Compositional Generalization Test\n";
    std::cout << "==================================\n\n";

    // Create network with attention enabled for better composition
    MLCConfig config;
    config.hidden_size = 128;
    config.embedding_dim = 64;
    config.attention_heads = 4;
    config.learning_rate = 0.001;
    config.use_attention = true;  // Critical for composition

    MLCNetwork network(config);

    // Generate episodes that teach primitives and test composition
    utils::EpisodeGenerator generator(42);

    std::cout << "Generating compositional training episodes...\n";
    std::vector<Episode> train_episodes;
    for (int i = 0; i < 500; ++i) {
        train_episodes.push_back(generator.generate_compositional_episode(5));
    }

    std::cout << "Generated " << train_episodes.size() << " episodes\n\n";

    // Train the network
    Trainer::TrainerConfig trainer_config;
    trainer_config.num_epochs = 50;
    trainer_config.batch_size = 32;
    trainer_config.eval_interval = 10;
    trainer_config.verbose = true;

    Trainer trainer(network, trainer_config);

    std::cout << "Training network on compositional tasks...\n";
    std::cout << std::string(60, '-') << "\n";
    trainer.train(train_episodes);

    // Test compositional generalization
    std::cout << "\n" << std::string(60, '-') << "\n";
    std::cout << "Testing Compositional Generalization:\n\n";

    auto test_episodes = generator.generate_batch(10, 5, 3);

    int correct = 0;
    for (size_t i = 0; i < test_episodes.size(); ++i) {
        const auto& ep = test_episodes[i];

        std::cout << "Test " << (i + 1) << ":\n";
        std::cout << "  Instruction: \"" << ep.query_example().instruction() << "\"\n";

        auto predicted = network.predict(
            ep.query_example().instruction(),
            ep.study_examples()
        );

        std::cout << "  Predicted: [";
        for (size_t j = 0; j < predicted.size(); ++j) {
            std::cout << predicted[j];
            if (j < predicted.size() - 1) std::cout << ", ";
        }
        std::cout << "]\n";

        std::cout << "  Expected:  [";
        for (size_t j = 0; j < ep.query_example().output().size(); ++j) {
            std::cout << ep.query_example().output()[j];
            if (j < ep.query_example().output().size() - 1) std::cout << ", ";
        }
        std::cout << "]\n";

        bool match = (predicted == ep.query_example().output());
        if (match) correct++;

        std::cout << "  Result: " << (match ? "✓ CORRECT" : "✗ INCORRECT") << "\n\n";
    }

    Float accuracy = static_cast<Float>(correct) / test_episodes.size();
    std::cout << std::string(60, '-') << "\n";
    std::cout << "Compositional Accuracy: " << std::fixed << std::setprecision(2)
              << (accuracy * 100.0) << "% (" << correct << "/" << test_episodes.size()
              << ")\n";

    return 0;
}
