#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cstddef>
#include <span>
#include <concepts>

namespace realm {

// Modern type aliases using C++20
using Float = float;
using DocumentId = size_t;
using QueryId = size_t;
using VectorDim = size_t;

// Concept for numeric types
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Dense vector representation
template<Numeric T = Float>
class DenseVector {
public:
    using ValueType = T;
    using Container = std::vector<T>;

    DenseVector() = default;
    explicit DenseVector(VectorDim dim, T init_value = T{0});
    DenseVector(std::initializer_list<T> values);
    explicit DenseVector(std::span<const T> data);

    // Accessors
    [[nodiscard]] VectorDim dimension() const noexcept { return data_.size(); }
    [[nodiscard]] std::span<const T> data() const noexcept { return data_; }
    [[nodiscard]] std::span<T> data() noexcept { return data_; }

    // Element access
    T& operator[](size_t idx) { return data_[idx]; }
    const T& operator[](size_t idx) const { return data_[idx]; }

    // Vector operations
    [[nodiscard]] T dot(const DenseVector& other) const;
    [[nodiscard]] T norm() const;
    void normalize();

private:
    Container data_;
};

// Document representation
struct Document {
    DocumentId id;
    std::string content;
    std::vector<std::string> tokens;

    Document() = default;
    Document(DocumentId id, std::string content);
};

// Query representation
struct Query {
    QueryId id;
    std::string content;
    std::vector<std::string> tokens;

    Query() = default;
    Query(QueryId id, std::string content);
};

// Hyperparameters configuration
struct HyperParams {
    // Training parameters
    size_t num_epochs{100};
    size_t batch_size{32};
    Float learning_rate{0.001f};

    // Model architecture
    VectorDim document_vector_size{768};
    VectorDim query_vector_size{768};
    VectorDim retriever_hidden_size{512};

    // Retrieval parameters
    size_t top_k_documents{5};

    // MIPS parameters
    size_t mips_param_m{10};
    Float mips_param_U{1.0f};
    Float mips_param_r{0.5f};

    // Regularization
    Float dropout_rate{0.1f};
    Float weight_decay{0.0001f};
};

// Training statistics
struct TrainingStats {
    Float mlm_loss{0.0f};
    Float retrieval_loss{0.0f};
    Float total_loss{0.0f};
    size_t epoch{0};

    void update(Float mlm, Float ret, size_t ep) {
        mlm_loss = mlm;
        retrieval_loss = ret;
        total_loss = mlm + ret;
        epoch = ep;
    }
};

} // namespace realm
