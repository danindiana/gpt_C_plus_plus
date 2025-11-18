/**
 * @file memory_example.cpp
 * @brief Example demonstrating memory-based novelty detection
 *
 * This example shows:
 * - Different distance metrics (Euclidean, Manhattan, Cosine, Chebyshev)
 * - K-nearest neighbor analysis
 * - Memory bank management
 * - Pattern recognition and novelty detection
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include "memory_detector.hpp"

void print_separator(char c = '=') {
    std::cout << std::string(70, c) << std::endl;
}

void print_vector(const std::string& label, const std::vector<float>& vec) {
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::fixed << std::setprecision(2) << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

int main() {
    std::cout << "Memory-Based Novelty Detection Example" << std::endl;
    print_separator();

    const int input_dim = 3;
    const size_t max_memory = 10;
    const float threshold = 0.5f;
    const int k = 3;  // 3-nearest neighbors

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Create detector
    std::cout << "\n1. Creating MemoryDetector..." << std::endl;
    MemoryDetector detector(
        input_dim,
        max_memory,
        threshold,
        k,
        MemoryDetector::DistanceMetric::EUCLIDEAN
    );

    std::cout << "   " << detector.get_config() << std::endl;

    // Generate some "known" patterns around specific centers
    std::cout << "\n2. Storing known patterns in memory..." << std::endl;

    std::vector<std::vector<float>> pattern_centers = {
        {0.2f, 0.2f, 0.2f},  // Pattern A: low values
        {0.5f, 0.5f, 0.5f},  // Pattern B: medium values
        {0.8f, 0.8f, 0.8f}   // Pattern C: high values
    };

    std::normal_distribution<float> noise(0.0f, 0.05f);

    for (size_t i = 0; i < pattern_centers.size(); ++i) {
        std::cout << "\n   Pattern " << char('A' + i) << " center: [";
        for (int j = 0; j < input_dim; ++j) {
            std::cout << std::fixed << std::setprecision(2) << pattern_centers[i][j];
            if (j < input_dim - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;

        // Add 3 variations of each pattern
        for (int var = 0; var < 3; ++var) {
            std::vector<float> pattern(input_dim);
            for (int j = 0; j < input_dim; ++j) {
                pattern[j] = pattern_centers[i][j] + noise(gen);
                pattern[j] = std::max(0.0f, std::min(1.0f, pattern[j]));  // Clamp [0,1]
            }
            detector.update_history(pattern);
            std::cout << "      Variation " << (var + 1) << ": [";
            for (int j = 0; j < input_dim; ++j) {
                std::cout << std::fixed << std::setprecision(2) << pattern[j];
                if (j < input_dim - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
    }

    std::cout << "\n   Memory bank size: " << detector.memory_count() << "/" << max_memory << std::endl;

    // Test with similar patterns (should have low novelty)
    print_separator();
    std::cout << "\n3. Testing with SIMILAR patterns..." << std::endl;

    for (size_t i = 0; i < pattern_centers.size(); ++i) {
        std::vector<float> test_pattern(input_dim);
        for (int j = 0; j < input_dim; ++j) {
            test_pattern[j] = pattern_centers[i][j] + noise(gen);
            test_pattern[j] = std::max(0.0f, std::min(1.0f, test_pattern[j]));
        }

        float score = detector.get_novelty_score(test_pattern);

        std::cout << "\n   Pattern " << char('A' + i) << " variant:" << std::endl;
        print_vector("      Input", test_pattern);
        std::cout << "      Novelty Score: " << std::fixed << std::setprecision(4)
                  << score << " (expected: low)" << std::endl;
    }

    // Test with novel patterns (should have high novelty)
    print_separator();
    std::cout << "\n4. Testing with NOVEL patterns..." << std::endl;

    std::vector<std::vector<float>> novel_patterns = {
        {0.0f, 1.0f, 0.0f},   // Novel: alternating pattern
        {1.0f, 0.0f, 1.0f},   // Novel: inverse alternating
        {0.1f, 0.9f, 0.5f}    // Novel: mixed pattern
    };

    for (size_t i = 0; i < novel_patterns.size(); ++i) {
        float score = detector.get_novelty_score(novel_patterns[i]);

        std::cout << "\n   Novel pattern " << (i + 1) << ":" << std::endl;
        print_vector("      Input", novel_patterns[i]);
        std::cout << "      Novelty Score: " << std::fixed << std::setprecision(4)
                  << score << " (expected: high)" << std::endl;

        if (score > 0.5f) {
            std::cout << "      ✓ Successfully detected as novel!" << std::endl;
        }
    }

    // Test different distance metrics
    print_separator();
    std::cout << "\n5. Comparing distance metrics..." << std::endl;

    std::vector<MemoryDetector::DistanceMetric> metrics = {
        MemoryDetector::DistanceMetric::EUCLIDEAN,
        MemoryDetector::DistanceMetric::MANHATTAN,
        MemoryDetector::DistanceMetric::COSINE,
        MemoryDetector::DistanceMetric::CHEBYSHEV
    };

    std::vector<std::string> metric_names = {
        "Euclidean", "Manhattan", "Cosine", "Chebyshev"
    };

    std::vector<float> test_pattern = {0.0f, 1.0f, 0.0f};
    print_vector("\n   Test pattern", test_pattern);

    std::cout << "\n   " << std::setw(15) << "Metric" << std::setw(20) << "Novelty Score" << std::endl;
    print_separator('-');

    for (size_t i = 0; i < metrics.size(); ++i) {
        MemoryDetector temp_detector(input_dim, max_memory, threshold, k, metrics[i]);

        // Rebuild memory
        for (const auto& center : pattern_centers) {
            for (int var = 0; var < 3; ++var) {
                std::vector<float> pattern(input_dim);
                for (int j = 0; j < input_dim; ++j) {
                    pattern[j] = center[j] + noise(gen);
                    pattern[j] = std::max(0.0f, std::min(1.0f, pattern[j]));
                }
                temp_detector.update_history(pattern);
            }
        }

        float score = temp_detector.get_novelty_score(test_pattern);
        std::cout << "   " << std::setw(15) << metric_names[i]
                  << std::setw(20) << std::fixed << std::setprecision(4) << score
                  << std::endl;
    }

    // Test K-nearest neighbor effect
    print_separator();
    std::cout << "\n6. Testing K-nearest neighbor effect..." << std::endl;

    print_vector("\n   Test pattern", test_pattern);

    std::cout << "\n   " << std::setw(10) << "K value" << std::setw(20) << "Novelty Score" << std::endl;
    print_separator('-');

    for (int test_k = 1; test_k <= 5; ++test_k) {
        MemoryDetector temp_detector(input_dim, max_memory, threshold, test_k);

        // Rebuild memory
        for (const auto& center : pattern_centers) {
            for (int var = 0; var < 3; ++var) {
                std::vector<float> pattern(input_dim);
                for (int j = 0; j < input_dim; ++j) {
                    pattern[j] = center[j] + noise(gen);
                    pattern[j] = std::max(0.0f, std::min(1.0f, pattern[j]));
                }
                temp_detector.update_history(pattern);
            }
        }

        float score = temp_detector.get_novelty_score(test_pattern);
        std::cout << "   " << std::setw(10) << test_k
                  << std::setw(20) << std::fixed << std::setprecision(4) << score
                  << std::endl;
    }

    // Memory management demonstration
    print_separator();
    std::cout << "\n7. Memory management (max size = " << max_memory << ")..." << std::endl;

    MemoryDetector mem_detector(input_dim, max_memory, threshold, k);
    mem_detector.set_online_learning(true);

    std::cout << "\n   Adding patterns beyond memory limit..." << std::endl;

    for (int i = 0; i < 15; ++i) {
        std::vector<float> pattern(input_dim);
        for (int j = 0; j < input_dim; ++j) {
            pattern[j] = dist(gen);
        }
        mem_detector.update_history(pattern);

        if (i % 3 == 0) {
            std::cout << "      After " << (i + 1) << " patterns: memory size = "
                      << mem_detector.memory_count() << std::endl;
        }
    }

    std::cout << "\n   Final memory size: " << mem_detector.memory_count()
              << " (limited to " << max_memory << ")" << std::endl;

    print_separator();
    std::cout << "\nExample completed successfully!" << std::endl;

    return 0;
}
