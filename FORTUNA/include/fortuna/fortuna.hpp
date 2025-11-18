#pragma once

/**
 * @file fortuna.hpp
 * @brief Main header for the Fortuna Bayesian neural network library
 *
 * This header provides a convenient single-include interface to all
 * Fortuna functionality including:
 * - Core neural network components (Layer, Model, DataLoader)
 * - Bayesian inference methods (MAP, ADVI, Ensembles, etc.)
 * - Utility functions (activations, initializers)
 * - Uncertainty quantification tools
 */

// Core components
#include "core/types.hpp"
#include "core/layer.hpp"
#include "core/model.hpp"
#include "core/data_loader.hpp"

// Bayesian inference
#include "bayesian/prob_classifier.hpp"

// Utilities
#include "utils/activations.hpp"
#include "utils/initializers.hpp"

/**
 * @namespace fortuna
 * @brief Main namespace for the Fortuna library
 *
 * The Fortuna library provides modern C++20 implementations of
 * Bayesian neural networks for uncertainty quantification in
 * deep learning applications.
 *
 * @namespace fortuna::core
 * @brief Core neural network components
 *
 * @namespace fortuna::bayesian
 * @brief Bayesian inference methods and probabilistic classifiers
 *
 * @namespace fortuna::utils
 * @brief Utility functions and helpers
 */

namespace fortuna {

/**
 * @brief Library version information
 */
constexpr const char* VERSION = "1.0.0";
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;

/**
 * @brief Get library version string
 *
 * @return const char* Version string
 */
[[nodiscard]] constexpr const char* version() noexcept {
    return VERSION;
}

} // namespace fortuna
