#include "fortuna/utils/initializers.hpp"
#include <algorithm>
#include <stdexcept>

namespace fortuna::utils {

void Initializers::xavier_uniform(RealMatrix& weights, Size fan_in, Size fan_out,
                                   unsigned int seed) {
    std::mt19937 gen(seed);
    Real limit = std::sqrt(6.0 / (fan_in + fan_out));
    std::uniform_real_distribution<Real> dist(-limit, limit);

    for (Size i = 0; i < fan_in; ++i) {
        for (Size j = 0; j < fan_out; ++j) {
            weights[i][j] = dist(gen);
        }
    }
}

void Initializers::xavier_normal(RealMatrix& weights, Size fan_in, Size fan_out,
                                  unsigned int seed) {
    std::mt19937 gen(seed);
    Real std_dev = std::sqrt(2.0 / (fan_in + fan_out));
    std::normal_distribution<Real> dist(0.0, std_dev);

    for (Size i = 0; i < fan_in; ++i) {
        for (Size j = 0; j < fan_out; ++j) {
            weights[i][j] = dist(gen);
        }
    }
}

void Initializers::he_uniform(RealMatrix& weights, Size fan_in, Size fan_out,
                               unsigned int seed) {
    std::mt19937 gen(seed);
    Real limit = std::sqrt(6.0 / fan_in);
    std::uniform_real_distribution<Real> dist(-limit, limit);

    for (Size i = 0; i < fan_in; ++i) {
        for (Size j = 0; j < fan_out; ++j) {
            weights[i][j] = dist(gen);
        }
    }
}

void Initializers::he_normal(RealMatrix& weights, Size fan_in, Size fan_out,
                              unsigned int seed) {
    std::mt19937 gen(seed);
    Real std_dev = std::sqrt(2.0 / fan_in);
    std::normal_distribution<Real> dist(0.0, std_dev);

    for (Size i = 0; i < fan_in; ++i) {
        for (Size j = 0; j < fan_out; ++j) {
            weights[i][j] = dist(gen);
        }
    }
}

void Initializers::lecun_uniform(RealMatrix& weights, Size fan_in, Size fan_out,
                                  unsigned int seed) {
    std::mt19937 gen(seed);
    Real limit = std::sqrt(3.0 / fan_in);
    std::uniform_real_distribution<Real> dist(-limit, limit);

    for (Size i = 0; i < fan_in; ++i) {
        for (Size j = 0; j < fan_out; ++j) {
            weights[i][j] = dist(gen);
        }
    }
}

void Initializers::lecun_normal(RealMatrix& weights, Size fan_in, Size fan_out,
                                 unsigned int seed) {
    std::mt19937 gen(seed);
    Real std_dev = std::sqrt(1.0 / fan_in);
    std::normal_distribution<Real> dist(0.0, std_dev);

    for (Size i = 0; i < fan_in; ++i) {
        for (Size j = 0; j < fan_out; ++j) {
            weights[i][j] = dist(gen);
        }
    }
}

void Initializers::zeros(RealMatrix& weights, Size fan_in, Size fan_out) {
    for (Size i = 0; i < fan_in; ++i) {
        std::fill(weights[i].begin(), weights[i].end(), 0.0);
    }
}

void Initializers::constant(RealMatrix& weights, Size fan_in, Size fan_out, Real value) {
    for (Size i = 0; i < fan_in; ++i) {
        std::fill(weights[i].begin(), weights[i].end(), value);
    }
}

void Initializers::zero_bias(RealVector& biases, Size size) {
    std::fill(biases.begin(), biases.end(), 0.0);
}

void Initializers::initialize(RealMatrix& weights, Size fan_in, Size fan_out,
                              const std::string& method, unsigned int seed) {
    if (method == "xavier" || method == "xavier_uniform" || method == "glorot") {
        xavier_uniform(weights, fan_in, fan_out, seed);
    } else if (method == "xavier_normal" || method == "glorot_normal") {
        xavier_normal(weights, fan_in, fan_out, seed);
    } else if (method == "he" || method == "he_uniform" || method == "kaiming") {
        he_uniform(weights, fan_in, fan_out, seed);
    } else if (method == "he_normal" || method == "kaiming_normal") {
        he_normal(weights, fan_in, fan_out, seed);
    } else if (method == "lecun" || method == "lecun_uniform") {
        lecun_uniform(weights, fan_in, fan_out, seed);
    } else if (method == "lecun_normal") {
        lecun_normal(weights, fan_in, fan_out, seed);
    } else if (method == "zeros" || method == "zero") {
        zeros(weights, fan_in, fan_out);
    } else {
        // Default to Xavier
        xavier_uniform(weights, fan_in, fan_out, seed);
    }
}

} // namespace fortuna::utils
