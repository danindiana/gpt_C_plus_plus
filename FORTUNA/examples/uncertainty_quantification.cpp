/**
 * @file uncertainty_quantification.cpp
 * @brief Demonstrates uncertainty quantification capabilities
 *
 * This example shows how to use different Bayesian methods for
 * uncertainty quantification in neural network predictions.
 */

#include <fortuna/fortuna.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>

using namespace fortuna;

/**
 * @brief Generate data with heteroscedastic noise
 *
 * Creates data where the noise level varies with input
 */
std::pair<RealMatrix, RealMatrix> generate_heteroscedastic_data(
    size_t num_samples, unsigned int seed = 42) {

    std::mt19937 gen(seed);
    std::uniform_real_distribution<Real> x_dist(-5.0, 5.0);

    RealMatrix inputs(num_samples, RealVector(1));
    RealMatrix targets(num_samples, RealVector(1));

    for (size_t i = 0; i < num_samples; ++i) {
        Real x = x_dist(gen);
        Real noise_std = 0.1 + 0.2 * std::abs(x) / 5.0;  // Increasing noise
        std::normal_distribution<Real> noise(0.0, noise_std);

        inputs[i][0] = x;
        targets[i][0] = 0.5 * x + noise(gen);
    }

    return {inputs, targets};
}

void compare_bayesian_methods() {
    std::cout << "=== Comparing Bayesian Methods ===\n\n";

    // List of Bayesian methods to compare
    std::vector<std::pair<BayesianMethod, std::string>> methods = {
        {BayesianMethod::MAP, "MAP (Maximum A Posteriori)"},
        {BayesianMethod::DEEP_ENSEMBLES, "Deep Ensembles"},
        {BayesianMethod::LAPLACE_GN, "Laplace Approximation"}
    };

    // Generate test data
    auto [inputs, targets] = generate_heteroscedastic_data(500);
    core::DataLoader data(inputs, targets, 32, true);
    auto [train_data, val_data] = core::DataLoader::train_val_split(data, 0.8);

    std::cout << "Training data samples: " << train_data.num_samples() << "\n";
    std::cout << "Validation data samples: " << val_data.num_samples() << "\n\n";

    // Test point for uncertainty comparison
    RealVector test_point = {2.5};

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Test point: x = " << test_point[0] << "\n\n";

    // Compare each method
    for (const auto& [method, name] : methods) {
        std::cout << "--- " << name << " ---\n";

        // Create model
        auto model = std::make_shared<core::Model>("UQ_Model");
        std::vector<Size> layer_sizes = {1, 16, 16, 1};
        model->build(layer_sizes, ActivationType::RELU, ActivationType::LINEAR);

        // Create classifier
        bayesian::ProbClassifier classifier(model, method);

        // Training config (shorter for demo)
        bayesian::TrainingConfig config;
        config.num_epochs = 20;
        config.learning_rate = 0.001;
        config.verbose = false;

        // Train
        classifier.train(train_data, val_data, config);

        // Predict with uncertainty
        auto result = classifier.predict(test_point, 100);

        std::cout << "  Mean prediction: " << result.mean[0] << "\n";
        std::cout << "  Std deviation: " << result.std[0] << "\n";
        std::cout << "  Epistemic uncertainty: " << result.epistemic_uncertainty << "\n";
        std::cout << "  Aleatoric uncertainty: " << result.aleatoric_uncertainty << "\n\n";
    }
}

void demonstrate_uncertainty_calibration() {
    std::cout << "\n=== Uncertainty Calibration Demo ===\n\n";

    // Generate data
    auto [inputs, targets] = generate_heteroscedastic_data(1000);
    core::DataLoader data(inputs, targets, 32, true);

    // Create and train model
    auto model = std::make_shared<core::Model>("Calibrated_Model");
    std::vector<Size> layer_sizes = {1, 32, 32, 1};
    model->build(layer_sizes, ActivationType::RELU, ActivationType::LINEAR);

    bayesian::ProbClassifier classifier(model, BayesianMethod::MAP);

    bayesian::TrainingConfig config;
    config.num_epochs = 50;
    config.learning_rate = 0.001;
    config.verbose = false;

    std::cout << "Training model for uncertainty calibration...\n";
    classifier.train(data, std::nullopt, config);

    // Demonstrate predictions at different x values
    std::cout << "\nPredictions across input range:\n";
    std::cout << "   x    | Prediction | Uncertainty\n";
    std::cout << "--------|------------|-----------\n";

    for (Real x = -5.0; x <= 5.0; x += 2.0) {
        RealVector input = {x};
        auto result = classifier.predict(input, 100);

        std::cout << std::setw(7) << x << " | "
                  << std::setw(10) << result.mean[0] << " | "
                  << std::setw(10) << result.std[0] << "\n";
    }

    std::cout << "\nNote: Uncertainty should increase for inputs far from training data.\n";
}

int main() {
    std::cout << "=== Fortuna Uncertainty Quantification Example ===\n\n";
    std::cout << "Fortuna version: " << fortuna::version() << "\n\n";

    std::cout << "This example demonstrates:\n";
    std::cout << "1. Comparison of different Bayesian methods\n";
    std::cout << "2. Uncertainty calibration\n";
    std::cout << "3. Epistemic vs Aleatoric uncertainty\n\n";

    // Run demonstrations
    compare_bayesian_methods();
    demonstrate_uncertainty_calibration();

    std::cout << "\n=== Uncertainty Quantification Example completed! ===\n";

    return 0;
}
