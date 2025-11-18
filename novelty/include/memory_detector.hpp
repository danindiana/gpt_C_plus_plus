#ifndef MEMORY_DETECTOR_HPP
#define MEMORY_DETECTOR_HPP

#include "novelty_detector.hpp"
#include "novelty_history.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>

/**
 * @brief Memory-based novelty detector using distance metrics
 *
 * This detector maintains a memory bank of observations and computes
 * novelty based on distance to stored memories. Novel inputs are those
 * that are far from all memorized patterns.
 *
 * Features:
 * - Multiple distance metrics (Euclidean, Manhattan, Cosine)
 * - K-nearest neighbor analysis
 * - Selective memory storage (only store novel patterns)
 */
class MemoryDetector : public NoveltyDetector {
public:
    /**
     * @brief Distance metric to use for comparison
     */
    enum class DistanceMetric {
        EUCLIDEAN,   // L2 distance
        MANHATTAN,   // L1 distance
        COSINE,      // Cosine similarity
        CHEBYSHEV    // L-infinity distance
    };

    /**
     * @brief Construct a new Memory Detector object
     * @param input_dim Input dimension
     * @param max_memory_size Maximum number of memories to store
     * @param distance_threshold Threshold for novelty
     * @param k Number of nearest neighbors to consider
     * @param metric Distance metric to use
     */
    explicit MemoryDetector(
        int input_dim,
        size_t max_memory_size = 50,
        float distance_threshold = 0.1f,
        int k = 1,
        DistanceMetric metric = DistanceMetric::EUCLIDEAN)
        : input_dim_(input_dim),
          max_memory_size_(max_memory_size),
          distance_threshold_(distance_threshold),
          k_(k),
          metric_(metric),
          history_(max_memory_size, input_dim) {

        if (input_dim <= 0) {
            throw std::invalid_argument("input_dim must be positive");
        }
        if (max_memory_size == 0) {
            throw std::invalid_argument("max_memory_size must be positive");
        }
        if (k <= 0 || k > static_cast<int>(max_memory_size)) {
            throw std::invalid_argument("k must be between 1 and max_memory_size");
        }

        memory_bank_.reserve(max_memory_size);
    }

    /**
     * @brief Forward pass - returns input scaled by novelty
     * @param input Input feature vector
     * @return Input scaled by novelty score
     */
    std::vector<float> forward(const std::vector<float>& input) override {
        validate_input(input);

        float novelty = get_novelty_score(input);

        std::vector<float> output(input_dim_);
        for (int i = 0; i < input_dim_; ++i) {
            output[i] = input[i] * novelty;
        }

        if (online_learning_) {
            update_history(input);
        }

        return output;
    }

    /**
     * @brief Get novelty score based on distance to memories
     * @param input Input feature vector
     * @return Novelty score (0.0 = seen before, 1.0 = completely novel)
     */
    float get_novelty_score(const std::vector<float>& input) override {
        validate_input(input);

        if (memory_bank_.empty()) {
            return 1.0f; // Everything is novel with empty memory
        }

        // Compute distances to all memories
        std::vector<float> distances = compute_all_distances(input);

        // Get k nearest neighbors
        std::vector<float> k_nearest = get_k_nearest(distances);

        // Average distance to k-nearest neighbors
        float avg_distance = std::accumulate(k_nearest.begin(), k_nearest.end(), 0.0f)
                           / k_nearest.size();

        // Normalize score
        return std::min(1.0f, avg_distance / distance_threshold_);
    }

    /**
     * @brief Update memory with new observation
     * @param input Input to potentially add to memory
     */
    void update_history(const std::vector<float>& input) override {
        validate_input(input);

        // Only add to memory if it's novel enough
        float novelty = get_novelty_score(input);

        if (novelty > 0.5f || memory_bank_.empty()) {
            add_to_memory(input);
        }

        // Always add to history for statistics
        history_.add(input);
    }

    /**
     * @brief Reset detector state
     */
    void reset() override {
        memory_bank_.clear();
        history_.clear();
    }

    /**
     * @brief Get configuration string
     * @return Configuration description
     */
    std::string get_config() const override {
        std::ostringstream oss;
        oss << "MemoryDetector(input_dim=" << input_dim_
            << ", max_memory=" << max_memory_size_
            << ", threshold=" << distance_threshold_
            << ", k=" << k_
            << ", metric=";

        switch (metric_) {
            case DistanceMetric::EUCLIDEAN:
                oss << "EUCLIDEAN";
                break;
            case DistanceMetric::MANHATTAN:
                oss << "MANHATTAN";
                break;
            case DistanceMetric::COSINE:
                oss << "COSINE";
                break;
            case DistanceMetric::CHEBYSHEV:
                oss << "CHEBYSHEV";
                break;
        }

        oss << ", memories=" << memory_bank_.size() << ")";
        return oss.str();
    }

    /**
     * @brief Set distance metric
     * @param metric New metric
     */
    void set_metric(DistanceMetric metric) {
        metric_ = metric;
    }

    /**
     * @brief Set distance threshold
     * @param threshold New threshold
     */
    void set_threshold(float threshold) {
        if (threshold <= 0.0f) {
            throw std::invalid_argument("Threshold must be positive");
        }
        distance_threshold_ = threshold;
    }

    /**
     * @brief Set k for k-nearest neighbors
     * @param k New k value
     */
    void set_k(int k) {
        if (k <= 0 || k > static_cast<int>(max_memory_size_)) {
            throw std::invalid_argument("k must be between 1 and max_memory_size");
        }
        k_ = k;
    }

    /**
     * @brief Get number of stored memories
     * @return Memory count
     */
    size_t memory_count() const {
        return memory_bank_.size();
    }

    /**
     * @brief Get read-only access to memory bank
     * @return Const reference to memory bank
     */
    const std::vector<std::vector<float>>& get_memory_bank() const {
        return memory_bank_;
    }

    /**
     * @brief Clear all memories
     */
    void clear_memory() {
        memory_bank_.clear();
    }

private:
    /**
     * @brief Validate input dimensions
     */
    void validate_input(const std::vector<float>& input) const {
        if (static_cast<int>(input.size()) != input_dim_) {
            throw std::invalid_argument(
                "Input size mismatch: expected " + std::to_string(input_dim_) +
                " but got " + std::to_string(input.size())
            );
        }
    }

    /**
     * @brief Add observation to memory bank
     */
    void add_to_memory(const std::vector<float>& input) {
        if (memory_bank_.size() >= max_memory_size_) {
            // Remove oldest memory (FIFO)
            memory_bank_.erase(memory_bank_.begin());
        }

        memory_bank_.push_back(input);
    }

    /**
     * @brief Compute distances to all memories
     */
    std::vector<float> compute_all_distances(const std::vector<float>& input) {
        std::vector<float> distances;
        distances.reserve(memory_bank_.size());

        for (const auto& memory : memory_bank_) {
            distances.push_back(compute_distance(input, memory));
        }

        return distances;
    }

    /**
     * @brief Get k nearest neighbor distances
     */
    std::vector<float> get_k_nearest(std::vector<float> distances) {
        int actual_k = std::min(k_, static_cast<int>(distances.size()));

        std::partial_sort(
            distances.begin(),
            distances.begin() + actual_k,
            distances.end()
        );

        return std::vector<float>(distances.begin(), distances.begin() + actual_k);
    }

    /**
     * @brief Compute distance between two vectors using selected metric
     */
    float compute_distance(const std::vector<float>& a,
                          const std::vector<float>& b) {
        switch (metric_) {
            case DistanceMetric::EUCLIDEAN:
                return euclidean_distance(a, b);

            case DistanceMetric::MANHATTAN:
                return manhattan_distance(a, b);

            case DistanceMetric::COSINE:
                return cosine_distance(a, b);

            case DistanceMetric::CHEBYSHEV:
                return chebyshev_distance(a, b);

            default:
                return euclidean_distance(a, b);
        }
    }

    /**
     * @brief Euclidean distance (L2)
     */
    static float euclidean_distance(const std::vector<float>& a,
                                   const std::vector<float>& b) {
        float sum = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            float diff = a[i] - b[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }

    /**
     * @brief Manhattan distance (L1)
     */
    static float manhattan_distance(const std::vector<float>& a,
                                   const std::vector<float>& b) {
        float sum = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            sum += std::abs(a[i] - b[i]);
        }
        return sum;
    }

    /**
     * @brief Cosine distance (1 - cosine similarity)
     */
    static float cosine_distance(const std::vector<float>& a,
                                const std::vector<float>& b) {
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;

        for (size_t i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }

        norm_a = std::sqrt(norm_a);
        norm_b = std::sqrt(norm_b);

        if (norm_a < 1e-6f || norm_b < 1e-6f) {
            return 1.0f; // Maximum distance for zero vectors
        }

        float similarity = dot / (norm_a * norm_b);
        return 1.0f - similarity;
    }

    /**
     * @brief Chebyshev distance (L-infinity)
     */
    static float chebyshev_distance(const std::vector<float>& a,
                                   const std::vector<float>& b) {
        float max_diff = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            max_diff = std::max(max_diff, std::abs(a[i] - b[i]));
        }
        return max_diff;
    }

    // Member variables
    int input_dim_;
    size_t max_memory_size_;
    float distance_threshold_;
    int k_;
    DistanceMetric metric_;
    std::vector<std::vector<float>> memory_bank_;
    NoveltyHistory history_;
};

#endif // MEMORY_DETECTOR_HPP
