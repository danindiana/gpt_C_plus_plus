#include "realm/types.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <sstream>

namespace realm {

// DenseVector implementation
template<Numeric T>
DenseVector<T>::DenseVector(VectorDim dim, T init_value)
    : data_(dim, init_value) {}

template<Numeric T>
DenseVector<T>::DenseVector(std::initializer_list<T> values)
    : data_(values) {}

template<Numeric T>
DenseVector<T>::DenseVector(std::span<const T> data)
    : data_(data.begin(), data.end()) {}

template<Numeric T>
T DenseVector<T>::dot(const DenseVector& other) const {
    if (dimension() != other.dimension()) {
        throw std::invalid_argument("Vector dimensions must match for dot product");
    }
    return std::inner_product(
        data_.begin(), data_.end(),
        other.data_.begin(),
        T{0}
    );
}

template<Numeric T>
T DenseVector<T>::norm() const {
    T sum_squares = std::inner_product(
        data_.begin(), data_.end(),
        data_.begin(),
        T{0}
    );
    return std::sqrt(sum_squares);
}

template<Numeric T>
void DenseVector<T>::normalize() {
    T n = norm();
    if (n > T{0}) {
        for (auto& val : data_) {
            val /= n;
        }
    }
}

// Explicit template instantiations
template class DenseVector<float>;
template class DenseVector<double>;

// Document implementation
Document::Document(DocumentId doc_id, std::string doc_content)
    : id(doc_id), content(std::move(doc_content)) {
    // Simple tokenization (split on whitespace)
    std::istringstream iss(this->content);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
}

// Query implementation
Query::Query(QueryId query_id, std::string query_content)
    : id(query_id), content(std::move(query_content)) {
    // Simple tokenization (split on whitespace)
    std::istringstream iss(this->content);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
}

} // namespace realm
