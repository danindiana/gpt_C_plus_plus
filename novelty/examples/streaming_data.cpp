/**
 * @file streaming_data.cpp
 * @brief Example demonstrating real-time novelty detection on streaming data
 *
 * This example simulates:
 * - Continuous data stream processing
 * - Online learning and adaptation
 * - Anomaly detection in time-series
 * - Real-time alerting
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <thread>
#include "novelty_attention.hpp"
#include "statistical_detector.hpp"

void print_separator(char c = '=') {
    std::cout << std::string(70, c) << std::endl;
}

/**
 * @brief Simulates a data stream with normal and anomalous periods
 */
class DataStreamSimulator {
public:
    DataStreamSimulator(int dim, unsigned seed = 42)
        : dim_(dim), gen_(seed),
          normal_dist_(50.0f, 5.0f),
          anomaly_dist_(100.0f, 10.0f) {}

    std::vector<float> next_sample() {
        std::vector<float> sample(dim_);

        // Inject anomalies at specific intervals
        bool is_anomaly = (sample_count_ % 50 == 0 && sample_count_ > 100);

        if (is_anomaly) {
            // Generate anomalous data
            for (int i = 0; i < dim_; ++i) {
                sample[i] = anomaly_dist_(gen_);
            }
            last_was_anomaly_ = true;
        } else {
            // Generate normal data with trend
            float trend = std::sin(sample_count_ * 0.1f) * 2.0f;
            for (int i = 0; i < dim_; ++i) {
                sample[i] = normal_dist_(gen_) + trend;
            }
            last_was_anomaly_ = false;
        }

        sample_count_++;
        return sample;
    }

    bool last_was_anomaly() const { return last_was_anomaly_; }
    int count() const { return sample_count_; }

    void reset() {
        sample_count_ = 0;
        last_was_anomaly_ = false;
    }

private:
    int dim_;
    std::mt19937 gen_;
    std::normal_distribution<float> normal_dist_;
    std::normal_distribution<float> anomaly_dist_;
    int sample_count_ = 0;
    bool last_was_anomaly_ = false;
};

/**
 * @brief Tracks detection statistics
 */
struct DetectionStats {
    int total_samples = 0;
    int true_positives = 0;   // Correctly detected anomalies
    int false_positives = 0;  // Normal data flagged as anomaly
    int true_negatives = 0;   // Correctly identified normal data
    int false_negatives = 0;  // Missed anomalies

    float precision() const {
        int predicted_positive = true_positives + false_positives;
        return predicted_positive > 0 ? float(true_positives) / predicted_positive : 0.0f;
    }

    float recall() const {
        int actual_positive = true_positives + false_negatives;
        return actual_positive > 0 ? float(true_positives) / actual_positive : 0.0f;
    }

    float f1_score() const {
        float p = precision();
        float r = recall();
        return (p + r) > 0 ? 2 * p * r / (p + r) : 0.0f;
    }

    float accuracy() const {
        return total_samples > 0 ?
            float(true_positives + true_negatives) / total_samples : 0.0f;
    }
};

void print_stats(const DetectionStats& stats) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "      Accuracy:  " << stats.accuracy() * 100 << "%" << std::endl;
    std::cout << "      Precision: " << stats.precision() * 100 << "%" << std::endl;
    std::cout << "      Recall:    " << stats.recall() * 100 << "%" << std::endl;
    std::cout << "      F1-Score:  " << stats.f1_score() << std::endl;
    std::cout << "      TP/FP/TN/FN: " << stats.true_positives << "/"
              << stats.false_positives << "/"
              << stats.true_negatives << "/"
              << stats.false_negatives << std::endl;
}

int main() {
    std::cout << "Streaming Data Novelty Detection Example" << std::endl;
    print_separator();

    const int input_dim = 5;
    const int stream_length = 500;
    const float novelty_threshold = 0.7f;

    // Create data stream simulator
    DataStreamSimulator stream(input_dim);

    std::cout << "\n1. Setting up streaming detection pipeline..." << std::endl;
    std::cout << "   Input dimension: " << input_dim << std::endl;
    std::cout << "   Stream length: " << stream_length << " samples" << std::endl;
    std::cout << "   Novelty threshold: " << novelty_threshold << std::endl;
    std::cout << "   Anomalies injected: every 50 samples (after warmup)" << std::endl;

    // Create detector with online learning
    StatisticalDetector detector(
        input_dim,
        100,  // sliding window
        2.0f,  // statistical threshold
        StatisticalDetector::Method::Z_SCORE
    );
    detector.set_online_learning(true);
    detector.set_learning_rate(0.1f);

    std::cout << "\n   " << detector.get_config() << std::endl;
    std::cout << "   Online learning: enabled" << std::endl;

    // Warmup period
    print_separator();
    std::cout << "\n2. Warmup period (100 samples)..." << std::endl;

    for (int i = 0; i < 100; ++i) {
        auto sample = stream.next_sample();
        detector.update_history(sample);

        if (i % 25 == 24) {
            std::cout << "   Processed " << (i + 1) << " samples..." << std::endl;
        }
    }

    std::cout << "   Warmup complete. Beginning real-time detection..." << std::endl;

    // Real-time detection
    print_separator();
    std::cout << "\n3. Real-time streaming detection..." << std::endl;

    DetectionStats stats;
    int alert_count = 0;

    std::cout << "\n   Time | Sample | Score  | Status     | Result" << std::endl;
    print_separator('-');

    for (int i = 0; i < stream_length - 100; ++i) {
        auto sample = stream.next_sample();
        float novelty_score = detector.get_novelty_score(sample);

        // Update detector (online learning)
        detector.update_history(sample);

        // Determine if this is actually an anomaly
        bool is_actual_anomaly = stream.last_was_anomaly();

        // Determine if we detected it as anomaly
        bool detected_as_anomaly = (novelty_score > novelty_threshold);

        // Update statistics
        stats.total_samples++;
        if (is_actual_anomaly && detected_as_anomaly) {
            stats.true_positives++;
        } else if (!is_actual_anomaly && detected_as_anomaly) {
            stats.false_positives++;
        } else if (!is_actual_anomaly && !detected_as_anomaly) {
            stats.true_negatives++;
        } else {
            stats.false_negatives++;
        }

        // Print updates for interesting events
        if (detected_as_anomaly || is_actual_anomaly) {
            std::cout << "   " << std::setw(4) << i
                      << " | " << std::setw(6) << stream.count()
                      << " | " << std::fixed << std::setprecision(3) << novelty_score;

            if (detected_as_anomaly) {
                std::cout << " | ALERT      |";
                alert_count++;
            } else {
                std::cout << " | normal     |";
            }

            if (is_actual_anomaly && detected_as_anomaly) {
                std::cout << " ✓ True Positive";
            } else if (is_actual_anomaly && !detected_as_anomaly) {
                std::cout << " ✗ False Negative";
            } else if (!is_actual_anomaly && detected_as_anomaly) {
                std::cout << " ⚠ False Positive";
            }

            std::cout << std::endl;
        }

        // Periodic status update
        if ((i + 1) % 100 == 0) {
            print_separator('-');
            std::cout << "   Status after " << (i + 1) << " samples:" << std::endl;
            std::cout << "      Alerts raised: " << alert_count << std::endl;
            std::cout << "      Current accuracy: " << std::fixed << std::setprecision(1)
                      << (stats.accuracy() * 100) << "%" << std::endl;
            print_separator('-');
        }
    }

    // Final statistics
    print_separator();
    std::cout << "\n4. Final Detection Statistics:" << std::endl;
    std::cout << "\n   Total samples processed: " << stats.total_samples << std::endl;
    std::cout << "   Total alerts raised: " << alert_count << std::endl;
    std::cout << "\n   Performance metrics:" << std::endl;
    print_stats(stats);

    // Performance evaluation
    print_separator();
    std::cout << "\n5. Performance Evaluation:" << std::endl;

    if (stats.accuracy() > 0.95f) {
        std::cout << "   ✓ Excellent: Accuracy > 95%" << std::endl;
    } else if (stats.accuracy() > 0.85f) {
        std::cout << "   ✓ Good: Accuracy > 85%" << std::endl;
    } else if (stats.accuracy() > 0.75f) {
        std::cout << "   ⚠ Fair: Accuracy > 75%" << std::endl;
    } else {
        std::cout << "   ✗ Poor: Accuracy < 75%" << std::endl;
    }

    if (stats.precision() > 0.9f && stats.recall() > 0.9f) {
        std::cout << "   ✓ High precision and recall" << std::endl;
    } else if (stats.precision() > 0.7f && stats.recall() > 0.7f) {
        std::cout << "   ✓ Moderate precision and recall" << std::endl;
    } else {
        std::cout << "   ⚠ Low precision or recall - consider tuning" << std::endl;
    }

    // Recommendations
    print_separator();
    std::cout << "\n6. Tuning Recommendations:" << std::endl;

    if (stats.false_positives > stats.false_negatives) {
        std::cout << "   • Consider increasing novelty threshold (reduce false positives)" << std::endl;
        std::cout << "     Current: " << novelty_threshold
                  << " → Try: " << (novelty_threshold + 0.1f) << std::endl;
    } else if (stats.false_negatives > stats.false_positives) {
        std::cout << "   • Consider decreasing novelty threshold (reduce false negatives)" << std::endl;
        std::cout << "     Current: " << novelty_threshold
                  << " → Try: " << (novelty_threshold - 0.1f) << std::endl;
    } else {
        std::cout << "   ✓ Threshold appears well-balanced" << std::endl;
    }

    if (stats.recall() < 0.8f) {
        std::cout << "   • Low recall: Consider decreasing statistical threshold" << std::endl;
    }

    if (stats.precision() < 0.8f) {
        std::cout << "   • Low precision: Consider increasing window size" << std::endl;
    }

    print_separator();
    std::cout << "\nKey Takeaways:" << std::endl;
    std::cout << "• Online learning allows adaptation to data trends" << std::endl;
    std::cout << "• Statistical methods work well for continuous streams" << std::endl;
    std::cout << "• Threshold tuning is critical for good performance" << std::endl;
    std::cout << "• Trade-off between false positives and false negatives" << std::endl;

    print_separator();
    std::cout << "\nExample completed successfully!" << std::endl;

    return 0;
}
