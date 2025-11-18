#ifndef NOVELTY_HISTORY_HPP
#define NOVELTY_HISTORY_HPP

#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <optional>

/**
 * @brief Statistics computed from historical observations
 */
struct Statistics {
    float mean = 0.0f;
    float std_dev = 0.0f;
    float min_value = 0.0f;
    float max_value = 0.0f;
    size_t count = 0;
};

/**
 * @brief Maintains history of observations for novelty detection
 *
 * This class provides a sliding window of historical observations
 * and computes various statistics useful for novelty detection.
 */
class NoveltyHistory {
public:
    /**
     * @brief Construct a new Novelty History object
     * @param max_size Maximum number of observations to store
     * @param dimension Dimension of each observation vector
     */
    explicit NoveltyHistory(size_t max_size = 1000, size_t dimension = 1)
        : max_size_(max_size), dimension_(dimension) {
        observations_.reserve(max_size);
    }

    /**
     * @brief Add a new observation to history
     * @param observation Vector to add
     */
    void add(const std::vector<float>& observation) {
        if (observation.size() != dimension_ && !observations_.empty()) {
            throw std::invalid_argument("Observation dimension mismatch");
        }

        if (observations_.empty()) {
            dimension_ = observation.size();
        }

        observations_.push_back(observation);

        // Maintain sliding window
        if (observations_.size() > max_size_) {
            observations_.pop_front();
        }

        // Invalidate cached statistics
        stats_valid_ = false;
    }

    /**
     * @brief Get statistics for a specific dimension
     * @param dim Dimension index
     * @return Statistics for that dimension
     */
    Statistics get_statistics(size_t dim = 0) const {
        if (observations_.empty()) {
            return Statistics{};
        }

        if (dim >= dimension_) {
            throw std::out_of_range("Dimension index out of range");
        }

        Statistics stats;
        stats.count = observations_.size();

        // Extract values for this dimension
        std::vector<float> values;
        values.reserve(observations_.size());
        for (const auto& obs : observations_) {
            values.push_back(obs[dim]);
        }

        // Compute statistics
        stats.min_value = *std::min_element(values.begin(), values.end());
        stats.max_value = *std::max_element(values.begin(), values.end());

        float sum = std::accumulate(values.begin(), values.end(), 0.0f);
        stats.mean = sum / values.size();

        float sq_sum = std::accumulate(values.begin(), values.end(), 0.0f,
            [stats](float acc, float val) {
                float diff = val - stats.mean;
                return acc + diff * diff;
            });

        stats.std_dev = std::sqrt(sq_sum / values.size());

        return stats;
    }

    /**
     * @brief Get global statistics across all dimensions
     * @return Global statistics
     */
    Statistics get_global_statistics() const {
        if (observations_.empty()) {
            return Statistics{};
        }

        Statistics stats;
        stats.count = observations_.size() * dimension_;

        std::vector<float> all_values;
        all_values.reserve(stats.count);

        for (const auto& obs : observations_) {
            all_values.insert(all_values.end(), obs.begin(), obs.end());
        }

        stats.min_value = *std::min_element(all_values.begin(), all_values.end());
        stats.max_value = *std::max_element(all_values.begin(), all_values.end());

        float sum = std::accumulate(all_values.begin(), all_values.end(), 0.0f);
        stats.mean = sum / all_values.size();

        float sq_sum = std::accumulate(all_values.begin(), all_values.end(), 0.0f,
            [stats](float acc, float val) {
                float diff = val - stats.mean;
                return acc + diff * diff;
            });

        stats.std_dev = std::sqrt(sq_sum / all_values.size());

        return stats;
    }

    /**
     * @brief Calculate distance to nearest historical observation
     * @param observation Query observation
     * @return Minimum Euclidean distance to history
     */
    float min_distance(const std::vector<float>& observation) const {
        if (observations_.empty()) {
            return std::numeric_limits<float>::max();
        }

        float min_dist = std::numeric_limits<float>::max();

        for (const auto& hist_obs : observations_) {
            float dist = euclidean_distance(observation, hist_obs);
            min_dist = std::min(min_dist, dist);
        }

        return min_dist;
    }

    /**
     * @brief Calculate average distance to all historical observations
     * @param observation Query observation
     * @return Average Euclidean distance
     */
    float average_distance(const std::vector<float>& observation) const {
        if (observations_.empty()) {
            return 0.0f;
        }

        float total_dist = 0.0f;

        for (const auto& hist_obs : observations_) {
            total_dist += euclidean_distance(observation, hist_obs);
        }

        return total_dist / observations_.size();
    }

    /**
     * @brief Clear all observations
     */
    void clear() {
        observations_.clear();
        stats_valid_ = false;
    }

    /**
     * @brief Get number of observations in history
     * @return Number of observations
     */
    size_t size() const { return observations_.size(); }

    /**
     * @brief Check if history is empty
     * @return True if empty
     */
    bool empty() const { return observations_.empty(); }

    /**
     * @brief Get dimension of observations
     * @return Dimension
     */
    size_t dimension() const { return dimension_; }

    /**
     * @brief Get maximum history size
     * @return Maximum size
     */
    size_t max_size() const { return max_size_; }

    /**
     * @brief Get read-only access to observations
     * @return Const reference to observations deque
     */
    const std::deque<std::vector<float>>& get_observations() const {
        return observations_;
    }

private:
    /**
     * @brief Calculate Euclidean distance between two vectors
     * @param a First vector
     * @param b Second vector
     * @return Euclidean distance
     */
    static float euclidean_distance(const std::vector<float>& a,
                                   const std::vector<float>& b) {
        if (a.size() != b.size()) {
            throw std::invalid_argument("Vector size mismatch");
        }

        float sum = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            float diff = a[i] - b[i];
            sum += diff * diff;
        }

        return std::sqrt(sum);
    }

    std::deque<std::vector<float>> observations_;
    size_t max_size_;
    size_t dimension_;
    mutable bool stats_valid_ = false;
};

#endif // NOVELTY_HISTORY_HPP
