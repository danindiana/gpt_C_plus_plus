/**
 * @file compile_test.cpp
 * @brief Basic compile-time test to ensure all headers work correctly
 */

#include <iostream>
#include <vector>
#include "novelty_detector.hpp"
#include "novelty_history.hpp"
#include "novelty_attention.hpp"
#include "statistical_detector.hpp"
#include "memory_detector.hpp"

int main() {
    std::cout << "=== Novelty Detection Framework - Compile Test ===" << std::endl;

    try {
        // Test NoveltyHistory
        {
            NoveltyHistory history(10, 3);
            std::vector<float> sample = {1.0f, 2.0f, 3.0f};
            history.add(sample);
            auto stats = history.get_global_statistics();
            std::cout << "✓ NoveltyHistory: OK (mean=" << stats.mean << ")" << std::endl;
        }

        // Test NoveltyAttention
        {
            NoveltyAttention attention(5, 2, 0.5f);
            std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
            auto output = attention.forward(input);
            float score = attention.get_novelty_score(input);
            std::cout << "✓ NoveltyAttention: OK (score=" << score << ")" << std::endl;
        }

        // Test StatisticalDetector
        {
            StatisticalDetector detector(5, 50, 2.0f);
            std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
            detector.update_history(input);
            float score = detector.get_novelty_score(input);
            std::cout << "✓ StatisticalDetector: OK (score=" << score << ")" << std::endl;
        }

        // Test MemoryDetector
        {
            MemoryDetector detector(5, 10, 0.5f, 3);
            std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
            detector.update_history(input);
            float score = detector.get_novelty_score(input);
            std::cout << "✓ MemoryDetector: OK (score=" << score << ")" << std::endl;
        }

        // Test different strategies
        {
            NoveltyAttention::ScoringStrategy strategies[] = {
                NoveltyAttention::ScoringStrategy::THRESHOLD,
                NoveltyAttention::ScoringStrategy::STATISTICAL,
                NoveltyAttention::ScoringStrategy::DISTANCE,
                NoveltyAttention::ScoringStrategy::ADAPTIVE
            };

            for (auto strategy : strategies) {
                NoveltyAttention detector(3, 1, 0.5f, 100, strategy);
                std::vector<float> input = {1.0f, 2.0f, 3.0f};
                detector.get_novelty_score(input);
            }
            std::cout << "✓ All scoring strategies: OK" << std::endl;
        }

        // Test different distance metrics
        {
            MemoryDetector::DistanceMetric metrics[] = {
                MemoryDetector::DistanceMetric::EUCLIDEAN,
                MemoryDetector::DistanceMetric::MANHATTAN,
                MemoryDetector::DistanceMetric::COSINE,
                MemoryDetector::DistanceMetric::CHEBYSHEV
            };

            for (auto metric : metrics) {
                MemoryDetector detector(3, 10, 0.5f, 2, metric);
                std::vector<float> input = {1.0f, 2.0f, 3.0f};
                detector.update_history(input);
            }
            std::cout << "✓ All distance metrics: OK" << std::endl;
        }

        // Test different statistical methods
        {
            StatisticalDetector::Method methods[] = {
                StatisticalDetector::Method::Z_SCORE,
                StatisticalDetector::Method::MODIFIED_Z_SCORE,
                StatisticalDetector::Method::PERCENTILE,
                StatisticalDetector::Method::IQR
            };

            for (auto method : methods) {
                StatisticalDetector detector(3, 50, 2.0f, method);
                std::vector<float> input = {1.0f, 2.0f, 3.0f};
                for (int i = 0; i < 10; ++i) {
                    detector.update_history(input);
                }
            }
            std::cout << "✓ All statistical methods: OK" << std::endl;
        }

        std::cout << "\n=== All Tests Passed! ===" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
