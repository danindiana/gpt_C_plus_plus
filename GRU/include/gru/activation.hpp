#pragma once

#include "types.hpp"
#include <cmath>
#include <algorithm>
#include <ranges>
#include <numbers>

namespace gru {

// Modern C++20 activation functions with concepts

template<Numeric T>
[[nodiscard]] constexpr T sigmoid(T x) noexcept {
    return static_cast<T>(1.0) / (static_cast<T>(1.0) + std::exp(-x));
}

template<Numeric T>
[[nodiscard]] constexpr T tanh_activation(T x) noexcept {
    return std::tanh(x);
}

template<Numeric T>
[[nodiscard]] constexpr T relu(T x) noexcept {
    return std::max(static_cast<T>(0.0), x);
}

// Vector-wise activation functions using ranges
[[nodiscard]] inline Vector sigmoid(const Vector& v) {
    Vector result;
    result.reserve(v.size());
    std::ranges::transform(v, std::back_inserter(result),
                          [](double x) { return sigmoid(x); });
    return result;
}

[[nodiscard]] inline Vector tanh_activation(const Vector& v) {
    Vector result;
    result.reserve(v.size());
    std::ranges::transform(v, std::back_inserter(result),
                          [](double x) { return tanh_activation(x); });
    return result;
}

[[nodiscard]] inline Vector relu(const Vector& v) {
    Vector result;
    result.reserve(v.size());
    std::ranges::transform(v, std::back_inserter(result),
                          [](double x) { return relu(x); });
    return result;
}

} // namespace gru
