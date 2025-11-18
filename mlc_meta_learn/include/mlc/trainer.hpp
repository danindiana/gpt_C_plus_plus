#pragma once

#include "types.hpp"
#include "mlc_network.hpp"
#include "episode.hpp"
#include "utils.hpp"
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace mlc {

/**
 * @brief Trainer for MLC networks
 *
 * Handles the training loop, logging, and checkpointing
 */
class Trainer {
public:
    struct TrainerConfig {
        std::size_t num_epochs = 100;
        std::size_t batch_size = 32;
        std::size_t eval_interval = 10;  // Evaluate every N epochs
        bool verbose = true;
        Float early_stop_threshold = 0.95;  // Stop if accuracy exceeds this
    };

    explicit Trainer(MLCNetwork& network, const TrainerConfig& config)
        : network_(network)
        , config_(config) {}

    explicit Trainer(MLCNetwork& network)
        : network_(network)
        , config_() {}

    /**
     * @brief Train the network on a dataset of episodes
     *
     * @param train_episodes Training episodes
     * @param val_episodes Validation episodes
     */
    void train(
        const std::vector<Episode>& train_episodes,
        const std::vector<Episode>& val_episodes = {}
    ) {
        if (train_episodes.empty()) {
            std::cerr << "Error: No training episodes provided\n";
            return;
        }

        auto start_time = std::chrono::steady_clock::now();

        for (std::size_t epoch = 0; epoch < config_.num_epochs; ++epoch) {
            network_.reset_stats();

            // Training phase
            Float epoch_loss = train_epoch(train_episodes, epoch);

            // Evaluation phase
            if (!val_episodes.empty() && epoch % config_.eval_interval == 0) {
                Float val_acc = network_.evaluate(val_episodes);

                if (config_.verbose) {
                    print_progress(epoch, epoch_loss, val_acc);
                }

                // Early stopping
                if (val_acc >= config_.early_stop_threshold) {
                    if (config_.verbose) {
                        std::cout << "\nEarly stopping: validation accuracy threshold reached\n";
                    }
                    break;
                }
            } else if (config_.verbose && epoch % config_.eval_interval == 0) {
                print_progress(epoch, epoch_loss, std::nullopt);
            }
        }

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            end_time - start_time
        ).count();

        if (config_.verbose) {
            std::cout << "\nTraining completed in " << duration << " seconds\n";
            print_final_stats();
        }
    }

    /**
     * @brief Evaluate the network on test episodes
     */
    Float evaluate(const std::vector<Episode>& test_episodes) {
        if (test_episodes.empty()) {
            std::cerr << "Error: No test episodes provided\n";
            return 0.0;
        }

        Float accuracy = network_.evaluate(test_episodes);

        if (config_.verbose) {
            std::cout << "Test Accuracy: " << std::fixed << std::setprecision(4)
                     << (accuracy * 100.0) << "%\n";
        }

        return accuracy;
    }

private:
    MLCNetwork& network_;
    TrainerConfig config_;

    Float train_epoch(const std::vector<Episode>& episodes, std::size_t /*epoch*/) {
        Float total_loss = 0.0;
        std::size_t num_batches = (episodes.size() + config_.batch_size - 1) / config_.batch_size;

        for (std::size_t batch = 0; batch < num_batches; ++batch) {
            std::size_t start_idx = batch * config_.batch_size;
            std::size_t end_idx = std::min(start_idx + config_.batch_size, episodes.size());

            for (std::size_t i = start_idx; i < end_idx; ++i) {
                total_loss += network_.train_on_episode(episodes[i]);
            }
        }

        return total_loss / episodes.size();
    }

    void print_progress(
        std::size_t epoch,
        Float loss,
        std::optional<Float> val_acc
    ) {
        std::cout << "Epoch " << std::setw(4) << epoch
                 << " | Loss: " << std::fixed << std::setprecision(6) << loss;

        if (val_acc.has_value()) {
            std::cout << " | Val Acc: " << std::setprecision(4)
                     << (val_acc.value() * 100.0) << "%";
        }

        const auto& stats = network_.stats();
        std::cout << " | Train Acc: " << std::setprecision(4)
                 << (stats.accuracy * 100.0) << "%\n";
    }

    void print_final_stats() {
        const auto& stats = network_.stats();
        std::cout << "\n=== Final Training Statistics ===\n"
                 << "Total Examples: " << stats.total_examples << "\n"
                 << "Correct Predictions: " << stats.correct_predictions << "\n"
                 << "Final Training Accuracy: " << std::fixed << std::setprecision(4)
                 << (stats.accuracy * 100.0) << "%\n"
                 << "Final Validation Accuracy: " << std::setprecision(4)
                 << (stats.validation_accuracy * 100.0) << "%\n";
    }
};

} // namespace mlc
