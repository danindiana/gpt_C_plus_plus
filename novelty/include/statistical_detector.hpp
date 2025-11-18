#ifndef STATISTICAL_DETECTOR_HPP
#define STATISTICAL_DETECTOR_HPP

#include "novelty_detector.hpp"
#include "novelty_history.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>

/**
 * @brief Statistical novelty detector using various statistical measures
 *
 * This detector uses statistical methods to identify outliers and novel patterns:
 * - Z-score analysis
 * - Modified Z-score (using median absolute deviation)
 * - Percentile-based detection
 * - Interquartile range (IQR) method
 */
class StatisticalDetector : public NoveltyDetector {
public:
    /**
     * @brief Statistical method to use for novelty detection
     */
    enum class Method {
        Z_SCORE,           // Standard Z-score
        MODIFIED_Z_SCORE,  // Median-based Z-score
        PERCENTILE,        // Percentile-based
        IQR                // Interquartile range
    };

    /**
     * @brief Construct a new Statistical Detector object
     * @param input_dim Input dimension
     * @param window_size Number of observations to maintain
     * @param threshold Statistical threshold (e.g., 2.0 for 2 std devs)
     * @param method Statistical method to use
     */
    explicit StatisticalDetector(
        int input_dim,
        size_t window_size = 100,
        float threshold = 2.0f,
        Method method = Method::Z_SCORE)
        : input_dim_(input_dim),
          threshold_(threshold),
          method_(method),
          history_(window_size, input_dim) {

        if (input_dim <= 0) {
            throw std::invalid_argument("input_dim must be positive");
        }
        if (threshold <= 0.0f) {
            throw std::invalid_argument("threshold must be positive");
        }
    }

    /**
     * @brief Forward pass - returns input with novelty highlighting
     * @param input Input feature vector
     * @return Input scaled by novelty scores
     */
    std::vector<float> forward(const std::vector<float>& input) override {
        validate_input(input);

        std::vector<float> output(input_dim_);
        auto novelty_scores = compute_all_scores(input);

        for (int i = 0; i < input_dim_; ++i) {
            output[i] = input[i] * novelty_scores[i];
        }

        if (online_learning_) {
            update_history(input);
        }

        return output;
    }

    /**
     * @brief Get overall novelty score
     * @param input Input feature vector
     * @return Maximum novelty score across all dimensions
     */
    float get_novelty_score(const std::vector<float>& input) override {
        validate_input(input);

        auto scores = compute_all_scores(input);
        return *std::max_element(scores.begin(), scores.end());
    }

    /**
     * @brief Update history with new observation
     * @param input Input to add to history
     */
    void update_history(const std::vector<float>& input) override {
        validate_input(input);
        history_.add(input);
    }

    /**
     * @brief Reset detector state
     */
    void reset() override {
        history_.clear();
    }

    /**
     * @brief Get configuration string
     * @return Configuration description
     */
    std::string get_config() const override {
        std::ostringstream oss;
        oss << "StatisticalDetector(input_dim=" << input_dim_
            << ", window=" << history_.max_size()
            << ", threshold=" << threshold_
            << ", method=";

        switch (method_) {
            case Method::Z_SCORE:
                oss << "Z_SCORE";
                break;
            case Method::MODIFIED_Z_SCORE:
                oss << "MODIFIED_Z_SCORE";
                break;
            case Method::PERCENTILE:
                oss << "PERCENTILE";
                break;
            case Method::IQR:
                oss << "IQR";
                break;
        }

        oss << ")";
        return oss.str();
    }

    /**
     * @brief Set statistical method
     * @param method New method
     */
    void set_method(Method method) {
        method_ = method;
    }

    /**
     * @brief Set threshold
     * @param threshold New threshold
     */
    void set_threshold(float threshold) {
        if (threshold <= 0.0f) {
            throw std::invalid_argument("Threshold must be positive");
        }
        threshold_ = threshold;
    }

    /**
     * @brief Get current threshold
     * @return Current threshold
     */
    float get_threshold() const {
        return threshold_;
    }

    /**
     * @brief Get per-dimension novelty scores
     * @param input Input vector
     * @return Vector of scores for each dimension
     */
    std::vector<float> get_dimension_scores(const std::vector<float>& input) {
        validate_input(input);
        return compute_all_scores(input);
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
     * @brief Compute novelty scores for all dimensions
     */
    std::vector<float> compute_all_scores(const std::vector<float>& input) {
        std::vector<float> scores(input_dim_, 0.0f);

        if (history_.empty()) {
            // No history - all data is novel
            std::fill(scores.begin(), scores.end(), 1.0f);
            return scores;
        }

        for (int i = 0; i < input_dim_; ++i) {
            scores[i] = compute_score(input[i], i);
        }

        return scores;
    }

    /**
     * @brief Compute novelty score for a single dimension
     */
    float compute_score(float value, int dimension) {
        switch (method_) {
            case Method::Z_SCORE:
                return compute_z_score(value, dimension);

            case Method::MODIFIED_Z_SCORE:
                return compute_modified_z_score(value, dimension);

            case Method::PERCENTILE:
                return compute_percentile_score(value, dimension);

            case Method::IQR:
                return compute_iqr_score(value, dimension);

            default:
                return compute_z_score(value, dimension);
        }
    }

    /**
     * @brief Standard Z-score calculation
     */
    float compute_z_score(float value, int dimension) {
        auto stats = history_.get_statistics(dimension);

        if (stats.std_dev < 1e-6f) {
            return 0.0f; // No variation
        }

        float z = std::abs(value - stats.mean) / stats.std_dev;
        return (z > threshold_) ? 1.0f : (z / threshold_);
    }

    /**
     * @brief Modified Z-score using median absolute deviation
     */
    float compute_modified_z_score(float value, int dimension) {
        if (history_.empty()) {
            return 1.0f;
        }

        // Extract values for this dimension
        std::vector<float> values;
        for (const auto& obs : history_.get_observations()) {
            values.push_back(obs[dimension]);
        }

        // Calculate median
        std::sort(values.begin(), values.end());
        float median = values[values.size() / 2];

        // Calculate median absolute deviation
        std::vector<float> abs_deviations;
        for (float v : values) {
            abs_deviations.push_back(std::abs(v - median));
        }
        std::sort(abs_deviations.begin(), abs_deviations.end());
        float mad = abs_deviations[abs_deviations.size() / 2];

        if (mad < 1e-6f) {
            return 0.0f;
        }

        // Modified Z-score
        float modified_z = 0.6745f * std::abs(value - median) / mad;
        return (modified_z > threshold_) ? 1.0f : (modified_z / threshold_);
    }

    /**
     * @brief Percentile-based scoring
     */
    float compute_percentile_score(float value, int dimension) {
        if (history_.empty()) {
            return 1.0f;
        }

        // Extract and sort values
        std::vector<float> values;
        for (const auto& obs : history_.get_observations()) {
            values.push_back(obs[dimension]);
        }
        std::sort(values.begin(), values.end());

        // Find percentile of current value
        auto it = std::lower_bound(values.begin(), values.end(), value);
        float percentile = static_cast<float>(it - values.begin()) / values.size();

        // Novel if in extreme percentiles
        float extreme = (100.0f - threshold_) / 100.0f;
        if (percentile < extreme || percentile > (1.0f - extreme)) {
            return 1.0f;
        }

        return 0.0f;
    }

    /**
     * @brief IQR (Interquartile Range) based scoring
     */
    float compute_iqr_score(float value, int dimension) {
        if (history_.size() < 4) {
            return 1.0f; // Need at least 4 samples
        }

        // Extract and sort values
        std::vector<float> values;
        for (const auto& obs : history_.get_observations()) {
            values.push_back(obs[dimension]);
        }
        std::sort(values.begin(), values.end());

        // Calculate Q1, Q3, and IQR
        size_t n = values.size();
        float q1 = values[n / 4];
        float q3 = values[3 * n / 4];
        float iqr = q3 - q1;

        if (iqr < 1e-6f) {
            return 0.0f;
        }

        // Outlier if beyond threshold * IQR from quartiles
        float lower_bound = q1 - threshold_ * iqr;
        float upper_bound = q3 + threshold_ * iqr;

        if (value < lower_bound || value > upper_bound) {
            return 1.0f;
        }

        // Compute distance to nearest boundary
        float dist_to_boundary = std::min(
            std::abs(value - lower_bound),
            std::abs(value - upper_bound)
        );

        return 1.0f - std::min(1.0f, dist_to_boundary / iqr);
    }

    // Member variables
    int input_dim_;
    float threshold_;
    Method method_;
    NoveltyHistory history_;
};

#endif // STATISTICAL_DETECTOR_HPP
