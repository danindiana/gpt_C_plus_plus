/**
 * @file ensemble_example.cpp
 * @brief Example demonstrating ensemble novelty detection
 *
 * This example shows how to combine multiple detectors for robust novelty detection:
 * - Attention-based detector
 * - Statistical detector
 * - Memory-based detector
 * - Weighted voting ensemble
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include "novelty_attention.hpp"
#include "statistical_detector.hpp"
#include "memory_detector.hpp"

void print_separator(char c = '=') {
    std::cout << std::string(70, c) << std::endl;
}

void print_vector(const std::string& label, const std::vector<float>& vec) {
    std::cout << label << ": [";
    for (size_t i = 0; i < std::min(vec.size(), size_t(5)); ++i) {
        std::cout << std::fixed << std::setprecision(2) << vec[i];
        if (i < std::min(vec.size(), size_t(5)) - 1) std::cout << ", ";
    }
    if (vec.size() > 5) std::cout << " ...";
    std::cout << "]" << std::endl;
}

/**
 * @brief Simple ensemble that combines multiple detectors
 */
class NoveltyEnsemble {
public:
    struct DetectorInfo {
        std::shared_ptr<NoveltyDetector> detector;
        float weight;
        std::string name;
    };

    void add_detector(std::shared_ptr<NoveltyDetector> detector,
                     float weight,
                     const std::string& name) {
        detectors_.push_back({detector, weight, name});
        normalize_weights();
    }

    float get_novelty_score(const std::vector<float>& input) {
        float total_score = 0.0f;

        for (auto& info : detectors_) {
            float score = info.detector->get_novelty_score(input);
            total_score += score * info.weight;
        }

        return total_score;
    }

    std::vector<float> get_individual_scores(const std::vector<float>& input) {
        std::vector<float> scores;
        for (auto& info : detectors_) {
            scores.push_back(info.detector->get_novelty_score(input));
        }
        return scores;
    }

    const std::vector<DetectorInfo>& get_detectors() const {
        return detectors_;
    }

    void update_all(const std::vector<float>& input) {
        for (auto& info : detectors_) {
            info.detector->update_history(input);
        }
    }

    void enable_online_learning(bool enable) {
        for (auto& info : detectors_) {
            info.detector->set_online_learning(enable);
        }
    }

private:
    void normalize_weights() {
        float sum = 0.0f;
        for (const auto& info : detectors_) {
            sum += info.weight;
        }
        if (sum > 0.0f) {
            for (auto& info : detectors_) {
                info.weight /= sum;
            }
        }
    }

    std::vector<DetectorInfo> detectors_;
};

std::vector<float> generate_normal_data(std::mt19937& gen, int dim) {
    std::normal_distribution<float> dist(50.0f, 10.0f);
    std::vector<float> data(dim);
    for (int i = 0; i < dim; ++i) {
        data[i] = dist(gen);
    }
    return data;
}

std::vector<float> generate_outlier_data(std::mt19937& gen, int dim) {
    std::uniform_real_distribution<float> dist(100.0f, 200.0f);
    std::vector<float> data(dim);
    for (int i = 0; i < dim; ++i) {
        data[i] = dist(gen);
    }
    return data;
}

int main() {
    std::cout << "Ensemble Novelty Detection Example" << std::endl;
    print_separator();

    const int input_dim = 10;
    std::random_device rd;
    std::mt19937 gen(rd());

    // Create ensemble
    std::cout << "\n1. Creating ensemble with multiple detectors..." << std::endl;

    NoveltyEnsemble ensemble;

    // Add attention-based detector
    auto attention = std::make_shared<NoveltyAttention>(
        input_dim, 3, 0.5f, 1000,
        NoveltyAttention::ScoringStrategy::ADAPTIVE
    );
    ensemble.add_detector(attention, 0.3f, "Attention");
    std::cout << "   Added: " << attention->get_config() << std::endl;
    std::cout << "          Weight: 0.30" << std::endl;

    // Add statistical detector
    auto statistical = std::make_shared<StatisticalDetector>(
        input_dim, 100, 2.0f,
        StatisticalDetector::Method::Z_SCORE
    );
    ensemble.add_detector(statistical, 0.4f, "Statistical");
    std::cout << "   Added: " << statistical->get_config() << std::endl;
    std::cout << "          Weight: 0.40" << std::endl;

    // Add memory-based detector
    auto memory = std::make_shared<MemoryDetector>(
        input_dim, 50, 0.2f, 3,
        MemoryDetector::DistanceMetric::EUCLIDEAN
    );
    ensemble.add_detector(memory, 0.3f, "Memory");
    std::cout << "   Added: " << memory->get_config() << std::endl;
    std::cout << "          Weight: 0.30" << std::endl;

    // Train ensemble
    print_separator();
    std::cout << "\n2. Training ensemble with normal data..." << std::endl;
    std::cout << "   Generating 100 training samples..." << std::endl;

    for (int i = 0; i < 100; ++i) {
        auto data = generate_normal_data(gen, input_dim);
        ensemble.update_all(data);
    }

    std::cout << "   Training complete!" << std::endl;

    // Test with normal data
    print_separator();
    std::cout << "\n3. Testing with NORMAL data..." << std::endl;

    auto normal_test = generate_normal_data(gen, input_dim);
    print_vector("   Input", normal_test);

    auto normal_individual = ensemble.get_individual_scores(normal_test);
    float normal_ensemble = ensemble.get_novelty_score(normal_test);

    std::cout << "\n   Individual detector scores:" << std::endl;
    const auto& detectors = ensemble.get_detectors();
    for (size_t i = 0; i < detectors.size(); ++i) {
        std::cout << "      " << std::setw(15) << detectors[i].name << ": "
                  << std::fixed << std::setprecision(4) << normal_individual[i]
                  << " (weight: " << std::setprecision(2) << detectors[i].weight << ")"
                  << std::endl;
    }

    std::cout << "\n   Ensemble score: " << std::fixed << std::setprecision(4)
              << normal_ensemble << " (expected: low)" << std::endl;

    // Test with outlier data
    print_separator();
    std::cout << "\n4. Testing with OUTLIER data..." << std::endl;

    auto outlier_test = generate_outlier_data(gen, input_dim);
    print_vector("   Input", outlier_test);

    auto outlier_individual = ensemble.get_individual_scores(outlier_test);
    float outlier_ensemble = ensemble.get_novelty_score(outlier_test);

    std::cout << "\n   Individual detector scores:" << std::endl;
    for (size_t i = 0; i < detectors.size(); ++i) {
        std::cout << "      " << std::setw(15) << detectors[i].name << ": "
                  << std::fixed << std::setprecision(4) << outlier_individual[i]
                  << " (weight: " << std::setprecision(2) << detectors[i].weight << ")"
                  << std::endl;
    }

    std::cout << "\n   Ensemble score: " << std::fixed << std::setprecision(4)
              << outlier_ensemble << " (expected: high)" << std::endl;

    // Compare results
    print_separator();
    std::cout << "\n5. Score comparison..." << std::endl;

    std::cout << "\n   " << std::setw(15) << "Detector"
              << std::setw(15) << "Normal"
              << std::setw(15) << "Outlier"
              << std::setw(15) << "Difference" << std::endl;
    print_separator('-');

    for (size_t i = 0; i < detectors.size(); ++i) {
        float diff = outlier_individual[i] - normal_individual[i];
        std::cout << "   " << std::setw(15) << detectors[i].name
                  << std::setw(15) << std::fixed << std::setprecision(4) << normal_individual[i]
                  << std::setw(15) << std::fixed << std::setprecision(4) << outlier_individual[i]
                  << std::setw(15) << std::fixed << std::setprecision(4) << diff
                  << std::endl;
    }

    float ensemble_diff = outlier_ensemble - normal_ensemble;
    print_separator('-');
    std::cout << "   " << std::setw(15) << "ENSEMBLE"
              << std::setw(15) << std::fixed << std::setprecision(4) << normal_ensemble
              << std::setw(15) << std::fixed << std::setprecision(4) << outlier_ensemble
              << std::setw(15) << std::fixed << std::setprecision(4) << ensemble_diff
              << std::endl;

    // Consensus analysis
    print_separator();
    std::cout << "\n6. Consensus analysis..." << std::endl;

    auto test_samples = {
        generate_normal_data(gen, input_dim),
        generate_normal_data(gen, input_dim),
        generate_outlier_data(gen, input_dim),
        generate_outlier_data(gen, input_dim)
    };

    std::cout << "\n   Sample  | Attention | Statistical | Memory  | Ensemble | Consensus" << std::endl;
    print_separator('-');

    int sample_num = 1;
    for (const auto& sample : test_samples) {
        auto scores = ensemble.get_individual_scores(sample);
        float ensemble_score = ensemble.get_novelty_score(sample);

        // Count how many detectors say it's novel (>0.5)
        int novel_votes = 0;
        for (float score : scores) {
            if (score > 0.5f) novel_votes++;
        }

        std::cout << "   " << std::setw(7) << sample_num
                  << " | " << std::setw(9) << std::fixed << std::setprecision(3) << scores[0]
                  << " | " << std::setw(11) << std::fixed << std::setprecision(3) << scores[1]
                  << " | " << std::setw(7) << std::fixed << std::setprecision(3) << scores[2]
                  << " | " << std::setw(8) << std::fixed << std::setprecision(3) << ensemble_score
                  << " | " << novel_votes << "/3";

        if (novel_votes >= 2) {
            std::cout << " ✓ NOVEL";
        } else {
            std::cout << " - normal";
        }
        std::cout << std::endl;

        sample_num++;
    }

    // Benefits of ensemble
    print_separator();
    std::cout << "\n7. Benefits of ensemble approach:" << std::endl;
    std::cout << "   ✓ Multiple perspectives on novelty" << std::endl;
    std::cout << "   ✓ Reduced false positives through consensus" << std::endl;
    std::cout << "   ✓ Robust to individual detector failures" << std::endl;
    std::cout << "   ✓ Complementary detection strategies" << std::endl;
    std::cout << "   ✓ Tunable via weight adjustment" << std::endl;

    if (ensemble_diff > 0.5f) {
        std::cout << "\n   ✓ Ensemble successfully distinguished normal from novel data!" << std::endl;
    }

    print_separator();
    std::cout << "\nExample completed successfully!" << std::endl;

    return 0;
}
