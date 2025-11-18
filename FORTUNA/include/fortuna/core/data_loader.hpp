#pragma once

#include "types.hpp"
#include <vector>
#include <span>
#include <optional>
#include <random>
#include <algorithm>

namespace fortuna::core {

/**
 * @brief Data batch structure
 */
struct Batch {
    RealMatrix inputs;   // [batch_size][input_dim]
    RealMatrix targets;  // [batch_size][output_dim]
    Size size;

    Batch(Size batch_size, Size input_dim, Size output_dim)
        : inputs(batch_size, RealVector(input_dim)),
          targets(batch_size, RealVector(output_dim)),
          size(batch_size) {}
};

/**
 * @brief Data loader class for handling training and validation data
 *
 * This class provides efficient data loading, batching, and shuffling
 * capabilities for training neural networks.
 */
class DataLoader {
public:
    /**
     * @brief Construct a new DataLoader object
     *
     * @param inputs Input data [num_samples][input_dim]
     * @param targets Target data [num_samples][output_dim]
     * @param batch_size Batch size for mini-batch training
     * @param shuffle Whether to shuffle data each epoch
     */
    DataLoader(RealMatrix inputs, RealMatrix targets,
               Size batch_size = 32, bool shuffle = true);

    /**
     * @brief Get the next batch of data
     *
     * @return std::optional<Batch> Batch if available, nullopt if epoch is complete
     */
    [[nodiscard]] std::optional<Batch> next_batch();

    /**
     * @brief Reset the data loader to start a new epoch
     */
    void reset();

    /**
     * @brief Get number of samples
     *
     * @return Size Total number of samples
     */
    [[nodiscard]] Size num_samples() const noexcept { return num_samples_; }

    /**
     * @brief Get number of batches per epoch
     *
     * @return Size Number of batches
     */
    [[nodiscard]] Size num_batches() const noexcept {
        return (num_samples_ + batch_size_ - 1) / batch_size_;
    }

    /**
     * @brief Get batch size
     *
     * @return Size Batch size
     */
    [[nodiscard]] Size batch_size() const noexcept { return batch_size_; }

    /**
     * @brief Get input dimension
     *
     * @return Size Input feature dimension
     */
    [[nodiscard]] Size input_dim() const noexcept { return input_dim_; }

    /**
     * @brief Get output dimension
     *
     * @return Size Output/target dimension
     */
    [[nodiscard]] Size output_dim() const noexcept { return output_dim_; }

    /**
     * @brief Get all input data
     *
     * @return const RealMatrix& Reference to input data
     */
    [[nodiscard]] const RealMatrix& inputs() const noexcept { return inputs_; }

    /**
     * @brief Get all target data
     *
     * @return const RealMatrix& Reference to target data
     */
    [[nodiscard]] const RealMatrix& targets() const noexcept { return targets_; }

    /**
     * @brief Split data into training and validation sets
     *
     * @param data Input data loader
     * @param train_ratio Ratio of data to use for training (0.0-1.0)
     * @param seed Random seed for shuffling
     * @return std::pair<DataLoader, DataLoader> Training and validation loaders
     */
    static std::pair<DataLoader, DataLoader> train_val_split(
        const DataLoader& data, Real train_ratio = 0.8, unsigned int seed = 42);

private:
    RealMatrix inputs_;
    RealMatrix targets_;
    std::vector<Size> indices_;
    Size num_samples_;
    Size batch_size_;
    Size input_dim_;
    Size output_dim_;
    Size current_index_;
    bool shuffle_;
    std::mt19937 rng_;

    /**
     * @brief Shuffle the data indices
     */
    void shuffle_indices();
};

} // namespace fortuna::core
