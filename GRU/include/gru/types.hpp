#pragma once

#include <vector>
#include <cstddef>
#include <span>
#include <concepts>

namespace gru {

// Type aliases for better readability
using Vector = std::vector<double>;
using Matrix = std::vector<std::vector<double>>;

// Concepts for type safety
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<typename T>
concept VectorLike = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
    { t[0] } -> std::convertible_to<double>;
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
};

} // namespace gru
