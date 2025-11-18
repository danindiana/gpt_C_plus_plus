/**
 * @file simple_classification.cpp
 * @brief Simple classification example using Fortuna library
 *
 * This example demonstrates basic usage of the Fortuna library for
 * a simple classification task with synthetic data.
 */

#include <fortuna/fortuna.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

using namespace fortuna;

/**
 * @brief Generate synthetic 2D classification data
 *
 * Creates a simple dataset with two classes separated by a circle
 */
std::pair<RealMatrix, RealMatrix> generate_data(size_t num_samples, unsigned int seed = 42) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<Real> angle_dist(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<Real> radius_dist(0.0, 2.0);
    std::normal_distribution<Real> noise_dist(0.0, 0.1);

    RealMatrix inputs(num_samples, RealVector(2));
    RealMatrix targets(num_samples, RealVector(1));

    for (size_t i = 0; i < num_samples; ++i) {
        Real angle = angle_dist(gen);
        Real radius = radius_dist(gen);

        inputs[i][0] = radius * std::cos(angle) + noise_dist(gen);
        inputs[i][1] = radius * std::sin(angle) + noise_dist(gen);

        // Class 0 if inside circle of radius 1, class 1 otherwise
        targets[i][0] = (inputs[i][0] * inputs[i][0] + inputs[i][1] * inputs[i][1] < 1.0) ? 0.0 : 1.0;
    }

    return {inputs, targets};
}

int main() {
    std::cout << "=== Fortuna Simple Classification Example ===\n\n";
    std::cout << "Fortuna version: " << fortuna::version() << "\n\n";

    // Generate synthetic data
    std::cout << "Generating synthetic data...\n";
    auto [inputs, targets] = generate_data(1000);

    // Create data loader
    core::DataLoader data(inputs, targets, 32, true);

    // Split into train and validation sets
    std::cout << "Splitting data into train/val sets...\n";
    auto [train_loader, val_loader] = core::DataLoader::train_val_split(data, 0.8);

    std::cout << "Training samples: " << train_loader.num_samples() << "\n";
    std::cout << "Validation samples: " << val_loader.num_samples() << "\n\n";

    // Create a neural network model
    std::cout << "Building neural network model...\n";
    auto model = std::make_shared<core::Model>("SimpleClassifier");

    // Architecture: 2 -> 16 -> 8 -> 1
    std::vector<Size> layer_sizes = {2, 16, 8, 1};
    model->build(layer_sizes, ActivationType::RELU, ActivationType::SIGMOID);

    std::cout << model->summary() << "\n";

    // Create Bayesian probabilistic classifier
    std::cout << "Creating Bayesian probabilistic classifier...\n";
    bayesian::ProbClassifier classifier(model, BayesianMethod::MAP);

    // Configure training
    bayesian::TrainingConfig config;
    config.num_epochs = 50;
    config.learning_rate = 0.001;
    config.batch_size = 32;
    config.verbose = true;
    config.print_every = 5;

    // Train the model
    std::cout << "\nTraining model...\n";
    classifier.train(train_loader, val_loader, config);

    // Make predictions on test data
    std::cout << "\n=== Testing Predictions ===\n";

    std::vector<std::vector<Real>> test_points = {
        {0.5, 0.5},   // Should be class 0 (inside circle)
        {1.5, 1.5},   // Should be class 1 (outside circle)
        {0.3, 0.3},   // Should be class 0
        {2.0, 0.0}    // Should be class 1
    };

    for (const auto& point : test_points) {
        auto prediction = classifier.predict_mean(point);
        std::cout << "Input: [" << point[0] << ", " << point[1] << "] "
                  << "-> Prediction: " << prediction[0]
                  << " (Class: " << (prediction[0] < 0.5 ? 0 : 1) << ")\n";
    }

    std::cout << "\n=== Example completed successfully! ===\n";

    return 0;
}
