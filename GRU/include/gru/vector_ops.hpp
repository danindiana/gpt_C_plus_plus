#pragma once

#include "types.hpp"
#include <algorithm>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <random>
#include <chrono>

namespace gru {

// Modern C++20 vector and matrix operations

// Vector addition
[[nodiscard]] inline Vector add(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Vector dimensions must match for addition");
    }

    Vector result;
    result.reserve(a.size());

    std::ranges::transform(a, b, std::back_inserter(result), std::plus<>{});
    return result;
}

// Vector subtraction
[[nodiscard]] inline Vector subtract(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Vector dimensions must match for subtraction");
    }

    Vector result;
    result.reserve(a.size());

    std::ranges::transform(a, b, std::back_inserter(result), std::minus<>{});
    return result;
}

// Element-wise multiplication (Hadamard product)
[[nodiscard]] inline Vector hadamard(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Vector dimensions must match for Hadamard product");
    }

    Vector result;
    result.reserve(a.size());

    std::ranges::transform(a, b, std::back_inserter(result), std::multiplies<>{});
    return result;
}

// Scalar multiplication
[[nodiscard]] inline Vector scale(const Vector& v, double scalar) {
    Vector result;
    result.reserve(v.size());

    std::ranges::transform(v, std::back_inserter(result),
                          [scalar](double x) { return x * scalar; });
    return result;
}

// Scalar addition to vector
[[nodiscard]] inline Vector add_scalar(const Vector& v, double scalar) {
    Vector result;
    result.reserve(v.size());

    std::ranges::transform(v, std::back_inserter(result),
                          [scalar](double x) { return x + scalar; });
    return result;
}

// Vector concatenation
[[nodiscard]] inline Vector concatenate(const Vector& a, const Vector& b) {
    Vector result;
    result.reserve(a.size() + b.size());

    std::ranges::copy(a, std::back_inserter(result));
    std::ranges::copy(b, std::back_inserter(result));

    return result;
}

// Matrix-vector multiplication
[[nodiscard]] inline Vector matmul(const Matrix& W, const Vector& x) {
    if (W.empty() || W[0].size() != x.size()) {
        throw std::invalid_argument("Matrix and vector dimensions are incompatible for multiplication");
    }

    Vector result(W.size(), 0.0);

    for (std::size_t i = 0; i < W.size(); ++i) {
        result[i] = std::inner_product(W[i].begin(), W[i].end(), x.begin(), 0.0);
    }

    return result;
}

// Xavier/Glorot initialization for better convergence
[[nodiscard]] inline Matrix xavier_init(std::size_t rows, std::size_t cols) {
    static std::mt19937_64 gen(static_cast<std::mt19937_64::result_type>(
        std::chrono::steady_clock::now().time_since_epoch().count()));

    const double limit = std::sqrt(6.0 / static_cast<double>(rows + cols));
    std::uniform_real_distribution<double> dist(-limit, limit);

    Matrix matrix(rows, Vector(cols));

    for (auto& row : matrix) {
        std::ranges::generate(row, [&]() { return dist(gen); });
    }

    return matrix;
}

// Random vector initialization
[[nodiscard]] inline Vector random_vector(std::size_t size, double mean = 0.0, double stddev = 0.1) {
    static std::mt19937_64 gen(static_cast<std::mt19937_64::result_type>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    std::normal_distribution<double> dist(mean, stddev);

    Vector vec(size);
    std::ranges::generate(vec, [&]() { return dist(gen); });

    return vec;
}

// Zero vector
[[nodiscard]] inline Vector zeros(std::size_t size) {
    return Vector(size, 0.0);
}

// Ones vector
[[nodiscard]] inline Vector ones(std::size_t size) {
    return Vector(size, 1.0);
}

// Complement vector (1 - v for each element)
[[nodiscard]] inline Vector complement(const Vector& v) {
    Vector result;
    result.reserve(v.size());

    std::ranges::transform(v, std::back_inserter(result),
                          [](double x) { return 1.0 - x; });
    return result;
}

} // namespace gru
