#include "mlc/mlc.hpp"
#include <iostream>
#include <iomanip>

using namespace mlc;

int main(int argc, char* argv[]) {
    std::cout << "=== MLC Framework Demo ===" << "\n";
    std::cout << "Meta-Learning for Compositionality v" << VERSION << "\n\n";

    try {
        // Configuration
        MLCConfig config;
        config.hidden_size = 128;
        config.embedding_dim = 64;
        config.attention_heads = 4;
        config.learning_rate = 0.001;
        config.batch_size = 32;
        config.use_attention = true;
        config.random_seed = 42;

        std::cout << "Configuration:\n"
                  << "  Hidden Size: " << config.hidden_size << "\n"
                  << "  Embedding Dim: " << config.embedding_dim << "\n"
                  << "  Attention Heads: " << config.attention_heads << "\n"
                  << "  Learning Rate: " << config.learning_rate << "\n"
                  << "  Batch Size: " << config.batch_size << "\n\n";

        // Create network
        std::cout << "Initializing MLC network...\n";
        MLCNetwork network(config);

        // Generate training data
        std::cout << "Generating training episodes...\n";
        utils::EpisodeGenerator generator(config.random_seed);

        std::size_t num_train = 1000;
        std::size_t num_val = 200;
        std::size_t num_test = 200;

        auto train_episodes = generator.generate_batch(num_train, 5, 2);
        auto val_episodes = generator.generate_batch(num_val, 5, 2);
        auto test_episodes = generator.generate_batch(num_test, 5, 2);

        std::cout << "  Training episodes: " << train_episodes.size() << "\n"
                  << "  Validation episodes: " << val_episodes.size() << "\n"
                  << "  Test episodes: " << test_episodes.size() << "\n\n";

        // Show example episode
        std::cout << "Example Episode:\n";
        if (!train_episodes.empty()) {
            const auto& example_ep = train_episodes[0];
            std::cout << "  Study Examples (" << example_ep.num_study_examples() << "):\n";
            for (std::size_t i = 0; i < std::min(size_t(3), example_ep.num_study_examples()); ++i) {
                const auto& ex = example_ep.study_examples()[i];
                std::cout << "    " << (i + 1) << ". \"" << ex.instruction() << "\" -> [";
                for (size_t j = 0; j < ex.output().size(); ++j) {
                    std::cout << ex.output()[j];
                    if (j < ex.output().size() - 1) std::cout << ", ";
                }
                std::cout << "]\n";
            }
            std::cout << "  Query: \"" << example_ep.query_example().instruction() << "\" -> [";
            for (size_t j = 0; j < example_ep.query_example().output().size(); ++j) {
                std::cout << example_ep.query_example().output()[j];
                if (j < example_ep.query_example().output().size() - 1) std::cout << ", ";
            }
            std::cout << "]\n\n";
        }

        // Train the network
        Trainer::TrainerConfig trainer_config;
        trainer_config.num_epochs = 100;
        trainer_config.batch_size = 32;
        trainer_config.eval_interval = 10;
        trainer_config.verbose = true;
        trainer_config.early_stop_threshold = 0.90;

        Trainer trainer(network, trainer_config);

        std::cout << "Starting training...\n";
        std::cout << std::string(60, '=') << "\n";
        trainer.train(train_episodes, val_episodes);

        // Final evaluation
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Final Evaluation on Test Set:\n";
        Float test_accuracy = trainer.evaluate(test_episodes);

        // Test compositional generalization
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Testing Compositional Generalization:\n\n";

        // Generate compositional test cases
        auto compositional_episodes = generator.generate_batch(5, 5, 3);
        for (const auto& ep : compositional_episodes) {
            std::cout << "Query: \"" << ep.query_example().instruction() << "\"\n";

            auto predicted = network.predict(
                ep.query_example().instruction(),
                ep.study_examples()
            );

            std::cout << "  Predicted: [";
            for (size_t i = 0; i < predicted.size(); ++i) {
                std::cout << predicted[i];
                if (i < predicted.size() - 1) std::cout << ", ";
            }
            std::cout << "]\n  Expected:  [";
            for (size_t i = 0; i < ep.query_example().output().size(); ++i) {
                std::cout << ep.query_example().output()[i];
                if (i < ep.query_example().output().size() - 1) std::cout << ", ";
            }
            std::cout << "]\n";

            bool correct = (predicted == ep.query_example().output());
            std::cout << "  " << (correct ? "✓ CORRECT" : "✗ INCORRECT") << "\n\n";
        }

        std::cout << "Demo completed successfully!\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
