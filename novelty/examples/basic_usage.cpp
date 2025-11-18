/**
 * @file basic_usage.cpp
 * @brief Basic usage example of the NoveltyAttention detector
 *
 * This example demonstrates:
 * - Creating a novelty detector
 * - Processing normal and novel data
 * - Computing novelty scores
 * - Using different scoring strategies
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include "novelty_attention.hpp"

void print_vector(const std::string& label, const std::vector<float>& vec) {
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::fixed << std::setprecision(2) << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

void print_separator() {
    std::cout << std::string(70, '=') << std::endl;
}

int main() {
    std::cout << "Novelty Detection - Basic Usage Example" << std::endl;
    print_separator();

    // Configuration
    const int input_dim = 10;
    const int num_heads = 3;
    const float threshold = 0.5f;

    // Create detector with different strategies
    std::cout << "\n1. Creating NoveltyAttention detector..." << std::endl;
    NoveltyAttention detector(
        input_dim,
        num_heads,
        threshold,
        1000,  // history size
        NoveltyAttention::ScoringStrategy::ADAPTIVE
    );

    std::cout << "   Configuration: " << detector.get_config() << std::endl;

    // Generate normal data (values in [0, 10])
    print_separator();
    std::cout << "\n2. Processing NORMAL data..." << std::endl;

    std::vector<float> normal_data = {
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
        6.0f, 7.0f, 8.0f, 9.0f, 10.0f
    };

    print_vector("   Input", normal_data);

    float normal_score = detector.get_novelty_score(normal_data);
    std::cout << "   Novelty Score: " << std::fixed << std::setprecision(4)
              << normal_score << " (low = familiar)" << std::endl;

    auto normal_output = detector.forward(normal_data);
    print_vector("   Output", normal_output);

    // Update history
    detector.update_history(normal_data);
    std::cout << "   History updated. Size: " << detector.get_history().size() << std::endl;

    // Process more normal data to build history
    std::cout << "\n3. Building history with more normal data..." << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> normal_dist(0.0f, 10.0f);

    for (int i = 0; i < 20; ++i) {
        std::vector<float> data(input_dim);
        for (int j = 0; j < input_dim; ++j) {
            data[j] = normal_dist(gen);
        }
        detector.update_history(data);
    }

    std::cout << "   History size: " << detector.get_history().size() << std::endl;

    // Now test with novel data (very different values)
    print_separator();
    std::cout << "\n4. Processing NOVEL data..." << std::endl;

    std::vector<float> novel_data = {
        100.0f, 200.0f, 300.0f, 400.0f, 500.0f,
        600.0f, 700.0f, 800.0f, 900.0f, 1000.0f
    };

    print_vector("   Input", novel_data);

    float novel_score = detector.get_novelty_score(novel_data);
    std::cout << "   Novelty Score: " << std::fixed << std::setprecision(4)
              << novel_score << " (high = novel!)" << std::endl;

    auto novel_output = detector.forward(novel_data);
    print_vector("   Output", novel_output);

    // Compare scores
    print_separator();
    std::cout << "\n5. Score Comparison:" << std::endl;
    std::cout << "   Normal data score: " << std::fixed << std::setprecision(4)
              << normal_score << std::endl;
    std::cout << "   Novel data score:  " << std::fixed << std::setprecision(4)
              << novel_score << std::endl;
    std::cout << "   Difference:        " << std::fixed << std::setprecision(4)
              << (novel_score - normal_score) << std::endl;

    if (novel_score > 0.7f) {
        std::cout << "\n   ✓ Novel pattern successfully detected!" << std::endl;
    }

    // Test different scoring strategies
    print_separator();
    std::cout << "\n6. Testing different scoring strategies..." << std::endl;

    std::vector<NoveltyAttention::ScoringStrategy> strategies = {
        NoveltyAttention::ScoringStrategy::THRESHOLD,
        NoveltyAttention::ScoringStrategy::STATISTICAL,
        NoveltyAttention::ScoringStrategy::DISTANCE,
        NoveltyAttention::ScoringStrategy::ADAPTIVE
    };

    std::vector<std::string> strategy_names = {
        "THRESHOLD", "STATISTICAL", "DISTANCE", "ADAPTIVE"
    };

    std::vector<float> test_data = {
        50.0f, 60.0f, 70.0f, 80.0f, 90.0f,
        100.0f, 110.0f, 120.0f, 130.0f, 140.0f
    };

    print_vector("\n   Test data", test_data);
    std::cout << std::endl;

    for (size_t i = 0; i < strategies.size(); ++i) {
        detector.set_strategy(strategies[i]);
        float score = detector.get_novelty_score(test_data);
        std::cout << "   " << std::setw(15) << std::left << strategy_names[i]
                  << " score: " << std::fixed << std::setprecision(4)
                  << score << std::endl;
    }

    // Get attention weights
    print_separator();
    std::cout << "\n7. Attention Weights:" << std::endl;
    detector.forward(test_data);
    auto weights = detector.get_attention_weights();
    print_vector("   Weights", weights);

    print_separator();
    std::cout << "\nExample completed successfully!" << std::endl;

    return 0;
}
