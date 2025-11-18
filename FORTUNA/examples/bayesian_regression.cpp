/**
 * @file bayesian_regression.cpp
 * @brief Bayesian regression example with Fortuna
 *
 * Demonstrates using Bayesian neural networks for regression tasks
 * with uncertainty quantification.
 */

#include <fortuna/fortuna.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>

using namespace fortuna;

/**
 * @brief Generate synthetic regression data
 *
 * Creates noisy data from a sine function
 */
std::pair<RealMatrix, RealMatrix> generate_regression_data(
    size_t num_samples, Real noise_level = 0.1, unsigned int seed = 42) {

    std::mt19937 gen(seed);
    std::normal_distribution<Real> noise(0.0, noise_level);
    std::uniform_real_distribution<Real> x_dist(-3.0, 3.0);

    RealMatrix inputs(num_samples, RealVector(1));
    RealMatrix targets(num_samples, RealVector(1));

    for (size_t i = 0; i < num_samples; ++i) {
        Real x = x_dist(gen);
        inputs[i][0] = x;
        targets[i][0] = std::sin(x) + noise(gen);
    }

    // Sort by x value for better visualization
    std::sort(inputs.begin(), inputs.end(),
              [](const RealVector& a, const RealVector& b) {
                  return a[0] < b[0];
              });

    // Recompute targets for sorted inputs
    std::mt19937 gen2(seed + 1);
    std::normal_distribution<Real> noise2(0.0, noise_level);
    for (size_t i = 0; i < num_samples; ++i) {
        Real x = inputs[i][0];
        targets[i][0] = std::sin(x) + noise2(gen2);
    }

    return {inputs, targets};
}

int main() {
    std::cout << "=== Fortuna Bayesian Regression Example ===\n\n";
    std::cout << "Fortuna version: " << fortuna::version() << "\n\n";

    // Generate synthetic regression data
    std::cout << "Generating synthetic regression data (y = sin(x) + noise)...\n";
    auto [inputs, targets] = generate_regression_data(200, 0.15);

    // Create data loader
    core::DataLoader data(inputs, targets, 32, true);

    // Split into train and validation sets
    auto [train_loader, val_loader] = core::DataLoader::train_val_split(data, 0.8);

    std::cout << "Training samples: " << train_loader.num_samples() << "\n";
    std::cout << "Validation samples: " << val_loader.num_samples() << "\n\n";

    // Create neural network model for regression
    std::cout << "Building neural network model...\n";
    auto model = std::make_shared<core::Model>("RegressionModel");

    // Architecture: 1 -> 32 -> 32 -> 1
    std::vector<Size> layer_sizes = {1, 32, 32, 1};
    model->build(layer_sizes, ActivationType::RELU, ActivationType::LINEAR);

    std::cout << model->summary() << "\n";

    // Create Bayesian probabilistic classifier
    bayesian::ProbClassifier regressor(model, BayesianMethod::MAP);

    // Configure training
    bayesian::TrainingConfig config;
    config.num_epochs = 100;
    config.learning_rate = 0.001;
    config.batch_size = 32;
    config.verbose = true;
    config.print_every = 10;

    // Train the model
    std::cout << "Training model...\n";
    regressor.train(train_loader, val_loader, config);

    // Make predictions with uncertainty
    std::cout << "\n=== Testing Predictions with Uncertainty ===\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "   x    | True y | Pred y | Std Dev\n";
    std::cout << "--------|--------|--------|--------\n";

    std::vector<Real> test_x = {-2.5, -1.5, -0.5, 0.0, 0.5, 1.5, 2.5};

    for (Real x : test_x) {
        RealVector input = {x};
        Real true_y = std::sin(x);
        auto result = regressor.predict(input, 100);

        std::cout << std::setw(7) << x << " | "
                  << std::setw(6) << true_y << " | "
                  << std::setw(6) << result.mean[0] << " | "
                  << std::setw(6) << result.std[0] << "\n";
    }

    // Analyze training history
    const auto& loss_history = regressor.loss_history();
    if (!loss_history.empty()) {
        std::cout << "\n=== Training Statistics ===\n";
        std::cout << "Initial loss: " << loss_history.front() << "\n";
        std::cout << "Final loss: " << loss_history.back() << "\n";
        std::cout << "Loss improvement: "
                  << (loss_history.front() - loss_history.back()) << "\n";
    }

    std::cout << "\n=== Bayesian Regression Example completed! ===\n";

    return 0;
}
