/**
 * @file statistical_example.cpp
 * @brief Example demonstrating statistical novelty detection methods
 *
 * This example shows:
 * - Different statistical methods (Z-score, Modified Z-score, IQR, Percentile)
 * - Outlier detection in time-series data
 * - Comparative analysis of methods
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include "statistical_detector.hpp"

void print_separator(char c = '=') {
    std::cout << std::string(70, c) << std::endl;
}

std::vector<float> generate_normal_sample(std::mt19937& gen, float mean, float stddev, int size) {
    std::normal_distribution<float> dist(mean, stddev);
    std::vector<float> sample(size);
    for (int i = 0; i < size; ++i) {
        sample[i] = dist(gen);
    }
    return sample;
}

int main() {
    std::cout << "Statistical Novelty Detection Example" << std::endl;
    print_separator();

    // Setup
    const int input_dim = 5;
    const size_t window_size = 100;
    const float threshold = 2.0f;  // 2 standard deviations

    std::random_device rd;
    std::mt19937 gen(rd());

    // Test different statistical methods
    std::vector<StatisticalDetector::Method> methods = {
        StatisticalDetector::Method::Z_SCORE,
        StatisticalDetector::Method::MODIFIED_Z_SCORE,
        StatisticalDetector::Method::IQR,
        StatisticalDetector::Method::PERCENTILE
    };

    std::vector<std::string> method_names = {
        "Z-Score", "Modified Z-Score", "IQR", "Percentile"
    };

    // Generate training data (normal distribution, mean=50, std=10)
    std::cout << "\n1. Generating training data..." << std::endl;
    std::cout << "   Distribution: Normal(mean=50, std=10)" << std::endl;
    std::cout << "   Samples: 100" << std::endl;

    std::vector<std::vector<float>> training_data;
    for (int i = 0; i < 100; ++i) {
        training_data.push_back(generate_normal_sample(gen, 50.0f, 10.0f, input_dim));
    }

    // Test each method
    for (size_t m = 0; m < methods.size(); ++m) {
        print_separator('-');
        std::cout << "\n" << (m + 2) << ". Testing: " << method_names[m] << std::endl;

        StatisticalDetector detector(input_dim, window_size, threshold, methods[m]);
        std::cout << "   " << detector.get_config() << std::endl;

        // Train detector
        std::cout << "\n   Training detector with normal data..." << std::endl;
        for (const auto& sample : training_data) {
            detector.update_history(sample);
        }

        // Test with normal data
        std::cout << "\n   Testing with NORMAL data:" << std::endl;
        auto normal_test = generate_normal_sample(gen, 50.0f, 10.0f, input_dim);
        float normal_score = detector.get_novelty_score(normal_test);

        std::cout << "      Sample: [";
        for (int i = 0; i < input_dim; ++i) {
            std::cout << std::fixed << std::setprecision(1) << normal_test[i];
            if (i < input_dim - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        std::cout << "      Novelty Score: " << std::fixed << std::setprecision(4)
                  << normal_score << " (expected: low)" << std::endl;

        // Test with outlier data
        std::cout << "\n   Testing with OUTLIER data:" << std::endl;

        // Create outlier (mean=100, way outside training distribution)
        auto outlier_test = generate_normal_sample(gen, 100.0f, 5.0f, input_dim);
        float outlier_score = detector.get_novelty_score(outlier_test);

        std::cout << "      Sample: [";
        for (int i = 0; i < input_dim; ++i) {
            std::cout << std::fixed << std::setprecision(1) << outlier_test[i];
            if (i < input_dim - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        std::cout << "      Novelty Score: " << std::fixed << std::setprecision(4)
                  << outlier_score << " (expected: high)" << std::endl;

        // Interpretation
        std::cout << "\n   Result: ";
        if (outlier_score > 0.7f && normal_score < 0.3f) {
            std::cout << "✓ Successfully distinguished outliers from normal data!" << std::endl;
        } else {
            std::cout << "⚠ Marginal separation between normal and outlier data" << std::endl;
        }
    }

    // Comparative analysis
    print_separator();
    std::cout << "\n" << (methods.size() + 2) << ". Comparative Analysis" << std::endl;
    std::cout << "\n   Testing all methods with the same data..." << std::endl;

    // Generate test samples
    auto normal_sample = generate_normal_sample(gen, 50.0f, 10.0f, input_dim);
    auto moderate_outlier = generate_normal_sample(gen, 80.0f, 5.0f, input_dim);
    auto extreme_outlier = generate_normal_sample(gen, 150.0f, 5.0f, input_dim);

    std::cout << "\n   " << std::setw(20) << "Method"
              << std::setw(15) << "Normal"
              << std::setw(15) << "Moderate"
              << std::setw(15) << "Extreme" << std::endl;
    print_separator('-');

    for (size_t m = 0; m < methods.size(); ++m) {
        StatisticalDetector detector(input_dim, window_size, threshold, methods[m]);

        // Train
        for (const auto& sample : training_data) {
            detector.update_history(sample);
        }

        // Test
        float s1 = detector.get_novelty_score(normal_sample);
        float s2 = detector.get_novelty_score(moderate_outlier);
        float s3 = detector.get_novelty_score(extreme_outlier);

        std::cout << "   " << std::setw(20) << method_names[m]
                  << std::setw(15) << std::fixed << std::setprecision(4) << s1
                  << std::setw(15) << std::fixed << std::setprecision(4) << s2
                  << std::setw(15) << std::fixed << std::setprecision(4) << s3
                  << std::endl;
    }

    // Per-dimension analysis
    print_separator();
    std::cout << "\n" << (methods.size() + 3) << ". Per-Dimension Analysis" << std::endl;

    StatisticalDetector detector(input_dim, window_size, threshold);

    // Train
    for (const auto& sample : training_data) {
        detector.update_history(sample);
    }

    // Test with mixed data (some dimensions normal, some outliers)
    std::vector<float> mixed_data = {
        50.0f,   // normal
        100.0f,  // outlier
        45.0f,   // normal
        150.0f,  // extreme outlier
        52.0f    // normal
    };

    std::cout << "\n   Mixed test data: [";
    for (int i = 0; i < input_dim; ++i) {
        std::cout << std::fixed << std::setprecision(1) << mixed_data[i];
        if (i < input_dim - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    auto dim_scores = detector.get_dimension_scores(mixed_data);

    std::cout << "\n   Dimension scores:" << std::endl;
    for (int i = 0; i < input_dim; ++i) {
        std::cout << "      Dim " << i << ": "
                  << std::fixed << std::setprecision(4) << dim_scores[i];
        if (dim_scores[i] > 0.7f) {
            std::cout << " (OUTLIER)";
        } else if (dim_scores[i] > 0.3f) {
            std::cout << " (suspicious)";
        } else {
            std::cout << " (normal)";
        }
        std::cout << std::endl;
    }

    print_separator();
    std::cout << "\nExample completed successfully!" << std::endl;

    return 0;
}
