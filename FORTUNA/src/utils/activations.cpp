#include "fortuna/utils/activations.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace fortuna::utils {

RealVector Activations::softmax(const RealVector& x) {
    RealVector result(x.size());

    // Find max for numerical stability
    Real max_val = *std::max_element(x.begin(), x.end());

    // Compute exp(x - max)
    std::transform(x.begin(), x.end(), result.begin(),
                  [max_val](Real val) { return std::exp(val - max_val); });

    // Compute sum of exponentials
    Real sum = std::accumulate(result.begin(), result.end(), Real{0.0});

    // Normalize
    std::transform(result.begin(), result.end(), result.begin(),
                  [sum](Real val) { return val / sum; });

    return result;
}

Real Activations::apply(Real x, ActivationType type) {
    switch (type) {
        case ActivationType::SIGMOID:
            return sigmoid(x);
        case ActivationType::TANH:
            return tanh(x);
        case ActivationType::RELU:
            return relu(x);
        case ActivationType::LEAKY_RELU:
            return leaky_relu(x);
        case ActivationType::ELU:
            return elu(x);
        case ActivationType::LINEAR:
            return linear(x);
        default:
            return x;
    }
}

RealVector Activations::apply(const RealVector& x, ActivationType type) {
    if (type == ActivationType::SOFTMAX) {
        return softmax(x);
    }

    RealVector result(x.size());
    std::transform(x.begin(), x.end(), result.begin(),
                  [type](Real val) { return apply(val, type); });
    return result;
}

} // namespace fortuna::utils
