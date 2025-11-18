#pragma once

#include "types.hpp"
#include <vector>
#include <array>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>
#include <cmath>
#include <random>

namespace mlc {

/**
 * @brief Simple tensor implementation for neural network operations
 *
 * This is a lightweight tensor class for basic neural network operations.
 * For production use, consider using Eigen, Armadillo, or similar libraries.
 */
class Tensor {
public:
    // Constructors
    Tensor() = default;

    explicit Tensor(std::vector<std::size_t> shape, Float init_value = 0.0)
        : shape_(std::move(shape)) {
        size_ = std::accumulate(shape_.begin(), shape_.end(), 1UL, std::multiplies<>());
        data_.resize(size_, init_value);
    }

    Tensor(std::vector<std::size_t> shape, std::vector<Float> data)
        : shape_(std::move(shape))
        , data_(std::move(data)) {
        size_ = std::accumulate(shape_.begin(), shape_.end(), 1UL, std::multiplies<>());
        if (data_.size() != size_) {
            throw std::invalid_argument("Data size doesn't match tensor shape");
        }
    }

    // Copy and move semantics
    Tensor(const Tensor&) = default;
    Tensor(Tensor&&) noexcept = default;
    Tensor& operator=(const Tensor&) = default;
    Tensor& operator=(Tensor&&) noexcept = default;

    ~Tensor() = default;

    // Accessors
    [[nodiscard]] const std::vector<Float>& data() const noexcept {
        return data_;
    }

    [[nodiscard]] std::vector<Float>& data() noexcept {
        return data_;
    }

    [[nodiscard]] const std::vector<std::size_t>& shape() const noexcept {
        return shape_;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] std::size_t ndim() const noexcept {
        return shape_.size();
    }

    // Element access
    [[nodiscard]] Float& operator[](std::size_t idx) {
        return data_[idx];
    }

    [[nodiscard]] const Float& operator[](std::size_t idx) const {
        return data_[idx];
    }

    // Multi-dimensional indexing
    template<typename... Indices>
    [[nodiscard]] Float& at(Indices... indices) {
        return data_[compute_index(indices...)];
    }

    template<typename... Indices>
    [[nodiscard]] const Float& at(Indices... indices) const {
        return data_[compute_index(indices...)];
    }

    // Operations
    Tensor& operator+=(const Tensor& other) {
        if (shape_ != other.shape_) {
            throw std::invalid_argument("Tensor shapes must match for addition");
        }
        std::transform(data_.begin(), data_.end(), other.data_.begin(), data_.begin(),
            std::plus<>());
        return *this;
    }

    Tensor& operator-=(const Tensor& other) {
        if (shape_ != other.shape_) {
            throw std::invalid_argument("Tensor shapes must match for subtraction");
        }
        std::transform(data_.begin(), data_.end(), other.data_.begin(), data_.begin(),
            std::minus<>());
        return *this;
    }

    Tensor& operator*=(Float scalar) {
        std::transform(data_.begin(), data_.end(), data_.begin(),
            [scalar](Float val) { return val * scalar; });
        return *this;
    }

    [[nodiscard]] Tensor operator+(const Tensor& other) const {
        Tensor result = *this;
        result += other;
        return result;
    }

    [[nodiscard]] Tensor operator-(const Tensor& other) const {
        Tensor result = *this;
        result -= other;
        return result;
    }

    [[nodiscard]] Tensor operator*(Float scalar) const {
        Tensor result = *this;
        result *= scalar;
        return result;
    }

    // Utility methods
    void fill(Float value) {
        std::fill(data_.begin(), data_.end(), value);
    }

    void zero() {
        fill(0.0);
    }

    void randomize(Float min = -1.0, Float max = 1.0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<Float> dist(min, max);
        std::generate(data_.begin(), data_.end(), [&]() { return dist(gen); });
    }

    [[nodiscard]] Float sum() const {
        return std::accumulate(data_.begin(), data_.end(), 0.0);
    }

    [[nodiscard]] Float mean() const {
        return size_ > 0 ? sum() / static_cast<Float>(size_) : 0.0;
    }

    [[nodiscard]] Float max() const {
        if (data_.empty()) return 0.0;
        return *std::max_element(data_.begin(), data_.end());
    }

    [[nodiscard]] Float min() const {
        if (data_.empty()) return 0.0;
        return *std::min_element(data_.begin(), data_.end());
    }

    // Reshape
    Tensor reshape(std::vector<std::size_t> new_shape) const {
        std::size_t new_size = std::accumulate(new_shape.begin(), new_shape.end(),
            1UL, std::multiplies<>());
        if (new_size != size_) {
            throw std::invalid_argument("New shape must have same number of elements");
        }
        return Tensor(std::move(new_shape), data_);
    }

private:
    std::vector<std::size_t> shape_;
    std::vector<Float> data_;
    std::size_t size_ = 0;

    template<typename... Indices>
    std::size_t compute_index(Indices... indices) const {
        std::array<std::size_t, sizeof...(indices)> idx_array{static_cast<std::size_t>(indices)...};
        if (idx_array.size() != shape_.size()) {
            throw std::invalid_argument("Number of indices must match tensor dimensions");
        }
        std::size_t index = 0;
        std::size_t multiplier = 1;
        for (int i = static_cast<int>(shape_.size()) - 1; i >= 0; --i) {
            if (idx_array[i] >= shape_[i]) {
                throw std::out_of_range("Index out of bounds");
            }
            index += idx_array[i] * multiplier;
            multiplier *= shape_[i];
        }
        return index;
    }
};

// Activation functions
namespace activation {

inline Float sigmoid(Float x) {
    return 1.0 / (1.0 + std::exp(-x));
}

inline Float tanh_activation(Float x) {
    return std::tanh(x);
}

inline Float relu(Float x) {
    return std::max(0.0, x);
}

inline Tensor sigmoid(const Tensor& t) {
    Tensor result = t;
    std::transform(result.data().begin(), result.data().end(), result.data().begin(),
        [](Float x) { return sigmoid(x); });
    return result;
}

inline Tensor tanh(const Tensor& t) {
    Tensor result = t;
    std::transform(result.data().begin(), result.data().end(), result.data().begin(),
        [](Float x) { return tanh_activation(x); });
    return result;
}

inline Tensor relu(const Tensor& t) {
    Tensor result = t;
    std::transform(result.data().begin(), result.data().end(), result.data().begin(),
        [](Float x) { return relu(x); });
    return result;
}

inline Tensor softmax(const Tensor& t) {
    Tensor result = t;
    Float max_val = t.max();
    Float sum = 0.0;

    for (auto& val : result.data()) {
        val = std::exp(val - max_val);
        sum += val;
    }

    for (auto& val : result.data()) {
        val /= sum;
    }

    return result;
}

} // namespace activation

} // namespace mlc
