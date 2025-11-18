#ifndef NOVELTY_ATTENTION_HPP
#define NOVELTY_ATTENTION_HPP

#include "novelty_detector.hpp"
#include "novelty_history.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>

/**
 * @brief Novelty-based attention mechanism for detecting novel patterns
 *
 * This class implements an attention mechanism that assigns higher weights
 * to novel features in the input data. It maintains a history of observations
 * and uses multiple strategies to compute novelty scores.
 *
 * Features:
 * - Multi-head attention
 * - Adaptive thresholding
 * - Statistical novelty scoring
 * - History-based comparison
 */
class NoveltyAttention : public NoveltyDetector {
public:
    /**
     * @brief Novelty scoring strategy
     */
    enum class ScoringStrategy {
        THRESHOLD,      // Simple threshold-based
        STATISTICAL,    // Z-score based
        DISTANCE,       // Distance to historical mean
        ADAPTIVE        // Adaptive threshold
    };

    /**
     * @brief Construct a new Novelty Attention object
     * @param input_dim Input dimension
     * @param num_heads Number of attention heads
     * @param novelty_threshold Initial novelty threshold
     * @param history_size Maximum history size
     * @param strategy Scoring strategy to use
     */
    explicit NoveltyAttention(
        int input_dim,
        int num_heads = 1,
        float novelty_threshold = 0.5f,
        size_t history_size = 1000,
        ScoringStrategy strategy = ScoringStrategy::ADAPTIVE)
        : input_dim_(input_dim),
          num_heads_(num_heads),
          novelty_threshold_(novelty_threshold),
          initial_threshold_(novelty_threshold),
          strategy_(strategy),
          history_(history_size, input_dim) {

        if (input_dim <= 0) {
            throw std::invalid_argument("input_dim must be positive");
        }
        if (num_heads <= 0) {
            throw std::invalid_argument("num_heads must be positive");
        }
        if (novelty_threshold < 0.0f) {
            throw std::invalid_argument("novelty_threshold must be non-negative");
        }

        attention_weights_.resize(num_heads_, 0.0f);
    }

    /**
     * @brief Forward pass through the attention mechanism
     * @param input Input feature vector
     * @return Attention-weighted output
     */
    std::vector<float> forward(const std::vector<float>& input) override {
        validate_input(input);

        // Compute novelty scores for each dimension
        std::vector<float> novelty_scores = compute_all_novelty_scores(input);

        // Compute attention weights for each head
        compute_attention_weights(input, novelty_scores);

        // Compute output by applying attention weights
        std::vector<float> output(input_dim_, 0.0f);

        for (int i = 0; i < input_dim_; ++i) {
            for (int head = 0; head < num_heads_; ++head) {
                output[i] += attention_weights_[head] * input[i] * novelty_scores[i];
            }
            output[i] /= num_heads_;
        }

        // Update history if online learning is enabled
        if (online_learning_) {
            update_history(input);
        }

        return output;
    }

    /**
     * @brief Get overall novelty score for input
     * @param input Input feature vector
     * @return Novelty score (0.0 to 1.0+)
     */
    float get_novelty_score(const std::vector<float>& input) override {
        validate_input(input);

        auto novelty_scores = compute_all_novelty_scores(input);

        // Return mean novelty score
        return std::accumulate(novelty_scores.begin(), novelty_scores.end(), 0.0f)
               / novelty_scores.size();
    }

    /**
     * @brief Update history with new observation
     * @param input Input to add to history
     */
    void update_history(const std::vector<float>& input) override {
        validate_input(input);
        history_.add(input);

        // Update adaptive threshold if using adaptive strategy
        if (strategy_ == ScoringStrategy::ADAPTIVE) {
            update_adaptive_threshold();
        }
    }

    /**
     * @brief Reset detector state
     */
    void reset() override {
        history_.clear();
        attention_weights_.assign(num_heads_, 0.0f);
        novelty_threshold_ = initial_threshold_;
    }

    /**
     * @brief Get configuration string
     * @return Configuration description
     */
    std::string get_config() const override {
        std::ostringstream oss;
        oss << "NoveltyAttention(input_dim=" << input_dim_
            << ", num_heads=" << num_heads_
            << ", threshold=" << novelty_threshold_
            << ", history_size=" << history_.size()
            << ", strategy=";

        switch (strategy_) {
            case ScoringStrategy::THRESHOLD:
                oss << "THRESHOLD";
                break;
            case ScoringStrategy::STATISTICAL:
                oss << "STATISTICAL";
                break;
            case ScoringStrategy::DISTANCE:
                oss << "DISTANCE";
                break;
            case ScoringStrategy::ADAPTIVE:
                oss << "ADAPTIVE";
                break;
        }

        oss << ")";
        return oss.str();
    }

    /**
     * @brief Get current attention weights
     * @return Vector of attention weights
     */
    const std::vector<float>& get_attention_weights() const {
        return attention_weights_;
    }

    /**
     * @brief Set scoring strategy
     * @param strategy New strategy
     */
    void set_strategy(ScoringStrategy strategy) {
        strategy_ = strategy;
    }

    /**
     * @brief Set novelty threshold
     * @param threshold New threshold
     */
    void set_threshold(float threshold) {
        if (threshold < 0.0f) {
            throw std::invalid_argument("Threshold must be non-negative");
        }
        novelty_threshold_ = threshold;
    }

    /**
     * @brief Get current novelty threshold
     * @return Current threshold
     */
    float get_threshold() const {
        return novelty_threshold_;
    }

    /**
     * @brief Get history object
     * @return Reference to history
     */
    const NoveltyHistory& get_history() const {
        return history_;
    }

private:
    /**
     * @brief Validate input dimensions
     * @param input Input to validate
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
     * @brief Compute novelty scores for all dimensions
     * @param input Input vector
     * @return Vector of novelty scores
     */
    std::vector<float> compute_all_novelty_scores(const std::vector<float>& input) {
        std::vector<float> scores(input_dim_);

        for (int i = 0; i < input_dim_; ++i) {
            scores[i] = compute_novelty_score(input[i], i);
        }

        return scores;
    }

    /**
     * @brief Compute novelty score for a single value using selected strategy
     * @param value Input value
     * @param dimension Dimension index
     * @return Novelty score
     */
    float compute_novelty_score(float value, int dimension) {
        switch (strategy_) {
            case ScoringStrategy::THRESHOLD:
                return compute_threshold_score(value);

            case ScoringStrategy::STATISTICAL:
                return compute_statistical_score(value, dimension);

            case ScoringStrategy::DISTANCE:
                return compute_distance_score(value, dimension);

            case ScoringStrategy::ADAPTIVE:
                return compute_adaptive_score(value, dimension);

            default:
                return compute_threshold_score(value);
        }
    }

    /**
     * @brief Simple threshold-based scoring
     */
    float compute_threshold_score(float value) const {
        return (std::abs(value) > novelty_threshold_) ? 1.0f : 0.0f;
    }

    /**
     * @brief Statistical Z-score based scoring
     */
    float compute_statistical_score(float value, int dimension) {
        if (history_.empty()) {
            return compute_threshold_score(value);
        }

        auto stats = history_.get_statistics(dimension);

        if (stats.std_dev < 1e-6f) {
            return 0.0f; // No variation in history
        }

        float z_score = std::abs(value - stats.mean) / stats.std_dev;
        return std::tanh(z_score); // Normalize to [0, 1]
    }

    /**
     * @brief Distance-based scoring (compared to historical mean)
     */
    float compute_distance_score(float value, int dimension) {
        if (history_.empty()) {
            return compute_threshold_score(value);
        }

        auto stats = history_.get_statistics(dimension);
        float distance = std::abs(value - stats.mean);
        float range = stats.max_value - stats.min_value;

        if (range < 1e-6f) {
            return 0.0f;
        }

        return std::min(1.0f, distance / range);
    }

    /**
     * @brief Adaptive scoring with dynamic threshold
     */
    float compute_adaptive_score(float value, int dimension) {
        if (history_.empty()) {
            return compute_threshold_score(value);
        }

        auto stats = history_.get_statistics(dimension);

        // Adaptive threshold: mean + k * std_dev
        float adaptive_threshold = stats.mean + novelty_threshold_ * stats.std_dev;
        float deviation = std::abs(value - stats.mean);

        return (deviation > adaptive_threshold) ? 1.0f : (deviation / adaptive_threshold);
    }

    /**
     * @brief Compute attention weights for all heads
     */
    void compute_attention_weights(const std::vector<float>& input,
                                  const std::vector<float>& novelty_scores) {
        for (int head = 0; head < num_heads_; ++head) {
            float sum = 0.0f;

            for (int i = 0; i < input_dim_; ++i) {
                sum += novelty_scores[i] * input[i];
            }

            attention_weights_[head] = sum / input_dim_;
        }

        // Normalize attention weights using softmax
        softmax(attention_weights_);
    }

    /**
     * @brief Apply softmax normalization to weights
     */
    void softmax(std::vector<float>& weights) {
        if (weights.empty()) return;

        float max_val = *std::max_element(weights.begin(), weights.end());
        float sum = 0.0f;

        for (auto& w : weights) {
            w = std::exp(w - max_val); // Subtract max for numerical stability
            sum += w;
        }

        if (sum > 1e-6f) {
            for (auto& w : weights) {
                w /= sum;
            }
        }
    }

    /**
     * @brief Update adaptive threshold based on history statistics
     */
    void update_adaptive_threshold() {
        if (history_.size() < 10) {
            return; // Need enough samples
        }

        auto global_stats = history_.get_global_statistics();

        // Adapt threshold using exponential moving average
        float new_threshold = global_stats.std_dev * 2.0f; // 2 sigma rule
        novelty_threshold_ = (1.0f - learning_rate_) * novelty_threshold_ +
                            learning_rate_ * new_threshold;
    }

    // Member variables
    int input_dim_;
    int num_heads_;
    float novelty_threshold_;
    float initial_threshold_;
    ScoringStrategy strategy_;
    NoveltyHistory history_;
    std::vector<float> attention_weights_;
};

#endif // NOVELTY_ATTENTION_HPP
