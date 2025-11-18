#include "fortuna/core/data_loader.hpp"
#include <stdexcept>
#include <algorithm>
#include <numeric>

namespace fortuna::core {

DataLoader::DataLoader(RealMatrix inputs, RealMatrix targets,
                       Size batch_size, bool shuffle)
    : inputs_(std::move(inputs)),
      targets_(std::move(targets)),
      batch_size_(batch_size),
      current_index_(0),
      shuffle_(shuffle),
      rng_(std::random_device{}()) {

    if (inputs_.empty() || targets_.empty()) {
        throw std::invalid_argument("Empty input or target data");
    }

    num_samples_ = inputs_.size();

    if (targets_.size() != num_samples_) {
        throw std::invalid_argument("Input and target sample counts don't match");
    }

    input_dim_ = inputs_[0].size();
    output_dim_ = targets_[0].size();

    // Validate dimensions
    for (const auto& input : inputs_) {
        if (input.size() != input_dim_) {
            throw std::invalid_argument("Inconsistent input dimensions");
        }
    }

    for (const auto& target : targets_) {
        if (target.size() != output_dim_) {
            throw std::invalid_argument("Inconsistent target dimensions");
        }
    }

    // Initialize indices
    indices_.resize(num_samples_);
    std::iota(indices_.begin(), indices_.end(), 0);

    if (shuffle_) {
        shuffle_indices();
    }
}

std::optional<Batch> DataLoader::next_batch() {
    if (current_index_ >= num_samples_) {
        return std::nullopt;
    }

    Size actual_batch_size = std::min(batch_size_, num_samples_ - current_index_);
    Batch batch(actual_batch_size, input_dim_, output_dim_);

    for (Size i = 0; i < actual_batch_size; ++i) {
        Size idx = indices_[current_index_ + i];
        batch.inputs[i] = inputs_[idx];
        batch.targets[i] = targets_[idx];
    }

    batch.size = actual_batch_size;
    current_index_ += actual_batch_size;

    return batch;
}

void DataLoader::reset() {
    current_index_ = 0;
    if (shuffle_) {
        shuffle_indices();
    }
}

void DataLoader::shuffle_indices() {
    std::shuffle(indices_.begin(), indices_.end(), rng_);
}

std::pair<DataLoader, DataLoader> DataLoader::train_val_split(
    const DataLoader& data, Real train_ratio, unsigned int seed) {

    if (train_ratio <= 0.0 || train_ratio >= 1.0) {
        throw std::invalid_argument("Train ratio must be in (0, 1)");
    }

    Size total_samples = data.num_samples();
    Size train_samples = static_cast<Size>(total_samples * train_ratio);
    Size val_samples = total_samples - train_samples;

    // Create shuffled indices
    std::vector<Size> indices(total_samples);
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 gen(seed);
    std::shuffle(indices.begin(), indices.end(), gen);

    // Split data
    RealMatrix train_inputs, train_targets;
    RealMatrix val_inputs, val_targets;

    train_inputs.reserve(train_samples);
    train_targets.reserve(train_samples);
    val_inputs.reserve(val_samples);
    val_targets.reserve(val_samples);

    for (Size i = 0; i < train_samples; ++i) {
        Size idx = indices[i];
        train_inputs.push_back(data.inputs_[idx]);
        train_targets.push_back(data.targets_[idx]);
    }

    for (Size i = train_samples; i < total_samples; ++i) {
        Size idx = indices[i];
        val_inputs.push_back(data.inputs_[idx]);
        val_targets.push_back(data.targets_[idx]);
    }

    return {
        DataLoader(std::move(train_inputs), std::move(train_targets),
                   data.batch_size_, data.shuffle_),
        DataLoader(std::move(val_inputs), std::move(val_targets),
                   data.batch_size_, false)
    };
}

} // namespace fortuna::core
