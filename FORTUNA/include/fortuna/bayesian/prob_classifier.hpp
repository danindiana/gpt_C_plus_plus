#pragma once

#include "../core/types.hpp"
#include "../core/model.hpp"
#include "../core/data_loader.hpp"
#include <memory>
#include <optional>
#include <functional>

namespace fortuna::bayesian {

/**
 * @brief Training configuration structure
 */
struct TrainingConfig {
    Size num_epochs = 100;
    Real learning_rate = 0.001;
    Real weight_decay = 0.0001;
    Size batch_size = 32;
    bool verbose = true;
    Size print_every = 10;

    // Bayesian-specific parameters
    Real prior_std = 1.0;           // Prior standard deviation
    Size num_samples = 100;          // Number of posterior samples
    Size burnin = 50;                // Burn-in period for MCMC
    Real temperature = 1.0;          // Temperature for uncertainty calibration
};

/**
 * @brief Prediction result with uncertainty quantification
 */
struct PredictionResult {
    RealVector mean;                 // Predictive mean
    RealVector std;                  // Predictive standard deviation
    RealVector variance;             // Predictive variance
    std::optional<RealMatrix> samples;  // Posterior samples (if available)
    Real epistemic_uncertainty;      // Model uncertainty
    Real aleatoric_uncertainty;      // Data uncertainty
};

/**
 * @brief Probabilistic classifier using Bayesian neural networks
 *
 * This class provides a unified interface for training and inference
 * using various Bayesian methods for uncertainty quantification.
 */
class ProbClassifier {
public:
    /**
     * @brief Construct a new ProbClassifier
     *
     * @param model Base neural network model
     * @param method Bayesian inference method
     */
    explicit ProbClassifier(std::shared_ptr<core::Model> model,
                           BayesianMethod method = BayesianMethod::MAP);

    /**
     * @brief Train the Bayesian model
     *
     * @param train_data Training data loader
     * @param val_data Optional validation data loader
     * @param config Training configuration
     */
    void train(core::DataLoader& train_data,
              std::optional<std::reference_wrapper<core::DataLoader>> val_data,
              const TrainingConfig& config);

    /**
     * @brief Predict with uncertainty quantification
     *
     * @param inputs Input data
     * @param num_samples Number of samples for uncertainty estimation
     * @return PredictionResult Predictions with uncertainty estimates
     */
    [[nodiscard]] PredictionResult predict(std::span<const Real> inputs,
                                          Size num_samples = 100) const;

    /**
     * @brief Predict mean only (faster, no uncertainty)
     *
     * @param inputs Input data
     * @return RealVector Predictive mean
     */
    [[nodiscard]] RealVector predict_mean(std::span<const Real> inputs) const;

    /**
     * @brief Get the underlying model
     *
     * @return std::shared_ptr<core::Model> Shared pointer to model
     */
    [[nodiscard]] std::shared_ptr<core::Model> model() const noexcept {
        return model_;
    }

    /**
     * @brief Get the Bayesian method used
     *
     * @return BayesianMethod Current Bayesian method
     */
    [[nodiscard]] BayesianMethod method() const noexcept {
        return method_;
    }

    /**
     * @brief Set the Bayesian method
     *
     * @param method New Bayesian method
     */
    void set_method(BayesianMethod method) {
        method_ = method;
    }

    /**
     * @brief Compute calibration metrics
     *
     * @param data Test data loader
     * @return Real Calibration score
     */
    [[nodiscard]] Real compute_calibration(core::DataLoader& data) const;

    /**
     * @brief Get training history
     *
     * @return const std::vector<Real>& Training loss history
     */
    [[nodiscard]] const std::vector<Real>& loss_history() const noexcept {
        return loss_history_;
    }

private:
    std::shared_ptr<core::Model> model_;
    BayesianMethod method_;
    std::vector<Real> loss_history_;
    std::vector<Real> val_loss_history_;
    bool is_trained_ = false;

    // Method-specific training implementations
    void train_map(core::DataLoader& train_data,
                   std::optional<std::reference_wrapper<core::DataLoader>> val_data,
                   const TrainingConfig& config);

    void train_advi(core::DataLoader& train_data,
                    std::optional<std::reference_wrapper<core::DataLoader>> val_data,
                    const TrainingConfig& config);

    void train_deep_ensembles(core::DataLoader& train_data,
                             std::optional<std::reference_wrapper<core::DataLoader>> val_data,
                             const TrainingConfig& config);

    void train_laplace(core::DataLoader& train_data,
                      std::optional<std::reference_wrapper<core::DataLoader>> val_data,
                      const TrainingConfig& config);

    void train_swag(core::DataLoader& train_data,
                   std::optional<std::reference_wrapper<core::DataLoader>> val_data,
                   const TrainingConfig& config);

    void train_sghmc(core::DataLoader& train_data,
                    std::optional<std::reference_wrapper<core::DataLoader>> val_data,
                    const TrainingConfig& config);

    // Helper methods
    [[nodiscard]] Real compute_loss(const RealVector& predictions,
                                   const RealVector& targets) const;

    [[nodiscard]] RealVector compute_gradient(const RealVector& predictions,
                                             const RealVector& targets) const;
};

} // namespace fortuna::bayesian
