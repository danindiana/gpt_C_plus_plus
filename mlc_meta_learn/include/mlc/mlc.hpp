#pragma once

/**
 * @file mlc.hpp
 * @brief Main header file for the MLC (Meta-Learning for Compositionality) library
 *
 * This file includes all the necessary headers for using the MLC framework.
 * Just include this one file to get access to all MLC functionality.
 */

// Core types and concepts
#include "types.hpp"

// Data structures
#include "example.hpp"
#include "episode.hpp"

// Neural network components
#include "tensor.hpp"
#include "attention.hpp"
#include "mlc_network.hpp"

// Training and utilities
#include "trainer.hpp"
#include "utils.hpp"

/**
 * @namespace mlc
 * @brief Meta-Learning for Compositionality framework
 *
 * Based on the paper "Human-like systematic generalization through a
 * meta-learning neural network" by Lake & Baroni (2023).
 *
 * Key components:
 * - Example: Single instruction-output pair
 * - Episode: Collection of study examples + query example
 * - MLCNetwork: Neural network with attention for compositional learning
 * - Trainer: Training loop and evaluation
 * - EpisodeGenerator: Generate synthetic training data
 */
namespace mlc {
    // Version info is in types.hpp
}
