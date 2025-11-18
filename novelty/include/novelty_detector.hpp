#ifndef NOVELTY_DETECTOR_HPP
#define NOVELTY_DETECTOR_HPP

#include <vector>
#include <memory>
#include <string>
#include <optional>

/**
 * @brief Abstract base class for novelty detection mechanisms
 *
 * This interface defines the contract for all novelty detection strategies.
 * Implementations should compute novelty scores for input data and maintain
 * history for adaptive detection.
 */
class NoveltyDetector {
public:
    virtual ~NoveltyDetector() = default;

    /**
     * @brief Process input data and return transformed output
     * @param input Input feature vector
     * @return Processed output vector
     */
    virtual std::vector<float> forward(const std::vector<float>& input) = 0;

    /**
     * @brief Compute novelty score for input data
     * @param input Input feature vector
     * @return Novelty score (higher = more novel)
     */
    virtual float get_novelty_score(const std::vector<float>& input) = 0;

    /**
     * @brief Update internal history with new observation
     * @param input Input feature vector to add to history
     */
    virtual void update_history(const std::vector<float>& input) = 0;

    /**
     * @brief Reset detector state and clear history
     */
    virtual void reset() = 0;

    /**
     * @brief Get detector configuration as string
     * @return Configuration string
     */
    virtual std::string get_config() const = 0;

    /**
     * @brief Enable/disable online learning
     * @param enable True to enable online learning
     */
    virtual void set_online_learning(bool enable) { online_learning_ = enable; }

    /**
     * @brief Set learning rate for online adaptation
     * @param rate Learning rate (0.0 to 1.0)
     */
    virtual void set_learning_rate(float rate) { learning_rate_ = rate; }

protected:
    bool online_learning_ = false;
    float learning_rate_ = 0.01f;
};

#endif // NOVELTY_DETECTOR_HPP
