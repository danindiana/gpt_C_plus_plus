/**
 * @file xor_example.cpp
 * @brief XOR problem solved with Bayesian neural network
 *
 * The XOR problem is a classic example demonstrating that a single perceptron
 * cannot learn non-linearly separable functions. This example shows how a
 * simple multi-layer network can solve it.
 */

#include <fortuna/fortuna.hpp>
#include <iostream>
#include <iomanip>

using namespace fortuna;

int main() {
    std::cout << "=== Fortuna XOR Problem Example ===\n\n";
    std::cout << "Fortuna version: " << fortuna::version() << "\n\n";

    // XOR truth table
    RealMatrix inputs = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {1.0, 1.0}
    };

    RealMatrix targets = {
        {0.0},
        {1.0},
        {1.0},
        {0.0}
    };

    std::cout << "XOR Truth Table:\n";
    std::cout << "Input  | Target\n";
    std::cout << "-------|-------\n";
    for (size_t i = 0; i < inputs.size(); ++i) {
        std::cout << inputs[i][0] << " " << inputs[i][1] << "  |  "
                  << targets[i][0] << "\n";
    }
    std::cout << "\n";

    // Create data loader (small batch size for XOR)
    core::DataLoader train_data(inputs, targets, 4, true);

    // Create a simple neural network
    std::cout << "Building neural network model...\n";
    auto model = std::make_shared<core::Model>("XOR_Solver");

    // Architecture: 2 -> 4 -> 1 (small network for XOR)
    std::vector<Size> layer_sizes = {2, 4, 1};
    model->build(layer_sizes, ActivationType::TANH, ActivationType::SIGMOID);

    std::cout << model->summary() << "\n";

    // Create Bayesian classifier
    bayesian::ProbClassifier classifier(model, BayesianMethod::MAP);

    // Configure training
    bayesian::TrainingConfig config;
    config.num_epochs = 100;
    config.learning_rate = 0.01;
    config.batch_size = 4;
    config.verbose = true;
    config.print_every = 10;

    // Train the model
    std::cout << "Training model on XOR problem...\n";
    classifier.train(train_data, std::nullopt, config);

    // Test the trained model
    std::cout << "\n=== Testing Trained Model ===\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Input  | Target | Prediction | Correct?\n";
    std::cout << "-------|--------|------------|----------\n";

    for (size_t i = 0; i < inputs.size(); ++i) {
        auto prediction = classifier.predict_mean(inputs[i]);
        Real pred_value = prediction[0];
        Real target_value = targets[i][0];
        bool correct = std::abs(pred_value - target_value) < 0.5;

        std::cout << inputs[i][0] << " " << inputs[i][1] << "  |  "
                  << target_value << "    |  "
                  << pred_value << "   |  "
                  << (correct ? "✓" : "✗") << "\n";
    }

    // Test with uncertainty quantification
    std::cout << "\n=== Predictions with Uncertainty ===\n";
    std::cout << "Input  | Mean  | Std Dev\n";
    std::cout << "-------|-------|--------\n";

    for (size_t i = 0; i < inputs.size(); ++i) {
        auto result = classifier.predict(inputs[i], 100);
        std::cout << inputs[i][0] << " " << inputs[i][1] << "  |  "
                  << result.mean[0] << " |  "
                  << result.std[0] << "\n";
    }

    std::cout << "\n=== XOR Example completed successfully! ===\n";

    return 0;
}
