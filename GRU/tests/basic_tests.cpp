/**
 * @file basic_tests.cpp
 * @brief Basic test suite for GRU implementation
 *
 * This is a simple test framework without external dependencies.
 * For production use, consider Google Test or Catch2.
 */

#include "gru/gru.hpp"
#include "gru/vector_ops.hpp"
#include "gru/activation.hpp"
#include <iostream>
#include <cmath>
#include <cassert>
#include <string>

// Simple test framework
int test_count = 0;
int test_passed = 0;

#define TEST(name) \
    void test_##name(); \
    struct TestRegistrar_##name { \
        TestRegistrar_##name() { \
            std::cout << "Running test: " #name << "..."; \
            test_count++; \
            try { \
                test_##name(); \
                std::cout << " PASSED\n"; \
                test_passed++; \
            } catch (const std::exception& e) { \
                std::cout << " FAILED: " << e.what() << "\n"; \
            } \
        } \
    }; \
    static TestRegistrar_##name registrar_##name; \
    void test_##name()

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #a + " != " + #b); \
    }

#define ASSERT_NEAR(a, b, epsilon) \
    if (std::abs((a) - (b)) > (epsilon)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #a + " not near " + #b); \
    }

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #cond); \
    }

// Tests for activation functions
TEST(sigmoid_basic) {
    ASSERT_NEAR(gru::sigmoid(0.0), 0.5, 1e-6);
    ASSERT_NEAR(gru::sigmoid(100.0), 1.0, 1e-6);
    ASSERT_NEAR(gru::sigmoid(-100.0), 0.0, 1e-6);
}

TEST(tanh_basic) {
    ASSERT_NEAR(gru::tanh_activation(0.0), 0.0, 1e-6);
    ASSERT_NEAR(gru::tanh_activation(100.0), 1.0, 1e-6);
    ASSERT_NEAR(gru::tanh_activation(-100.0), -1.0, 1e-6);
}

TEST(relu_basic) {
    ASSERT_EQ(gru::relu(5.0), 5.0);
    ASSERT_EQ(gru::relu(-5.0), 0.0);
    ASSERT_EQ(gru::relu(0.0), 0.0);
}

// Tests for vector operations
TEST(vector_add) {
    gru::Vector a = {1.0, 2.0, 3.0};
    gru::Vector b = {4.0, 5.0, 6.0};
    auto result = gru::add(a, b);

    ASSERT_EQ(result.size(), 3);
    ASSERT_NEAR(result[0], 5.0, 1e-6);
    ASSERT_NEAR(result[1], 7.0, 1e-6);
    ASSERT_NEAR(result[2], 9.0, 1e-6);
}

TEST(vector_subtract) {
    gru::Vector a = {5.0, 7.0, 9.0};
    gru::Vector b = {1.0, 2.0, 3.0};
    auto result = gru::subtract(a, b);

    ASSERT_EQ(result.size(), 3);
    ASSERT_NEAR(result[0], 4.0, 1e-6);
    ASSERT_NEAR(result[1], 5.0, 1e-6);
    ASSERT_NEAR(result[2], 6.0, 1e-6);
}

TEST(vector_hadamard) {
    gru::Vector a = {2.0, 3.0, 4.0};
    gru::Vector b = {5.0, 6.0, 7.0};
    auto result = gru::hadamard(a, b);

    ASSERT_EQ(result.size(), 3);
    ASSERT_NEAR(result[0], 10.0, 1e-6);
    ASSERT_NEAR(result[1], 18.0, 1e-6);
    ASSERT_NEAR(result[2], 28.0, 1e-6);
}

TEST(vector_scale) {
    gru::Vector v = {1.0, 2.0, 3.0};
    auto result = gru::scale(v, 2.5);

    ASSERT_EQ(result.size(), 3);
    ASSERT_NEAR(result[0], 2.5, 1e-6);
    ASSERT_NEAR(result[1], 5.0, 1e-6);
    ASSERT_NEAR(result[2], 7.5, 1e-6);
}

TEST(vector_concatenate) {
    gru::Vector a = {1.0, 2.0};
    gru::Vector b = {3.0, 4.0, 5.0};
    auto result = gru::concatenate(a, b);

    ASSERT_EQ(result.size(), 5);
    ASSERT_NEAR(result[0], 1.0, 1e-6);
    ASSERT_NEAR(result[1], 2.0, 1e-6);
    ASSERT_NEAR(result[2], 3.0, 1e-6);
    ASSERT_NEAR(result[3], 4.0, 1e-6);
    ASSERT_NEAR(result[4], 5.0, 1e-6);
}

TEST(vector_complement) {
    gru::Vector v = {0.2, 0.5, 0.8};
    auto result = gru::complement(v);

    ASSERT_EQ(result.size(), 3);
    ASSERT_NEAR(result[0], 0.8, 1e-6);
    ASSERT_NEAR(result[1], 0.5, 1e-6);
    ASSERT_NEAR(result[2], 0.2, 1e-6);
}

TEST(matmul_basic) {
    gru::Matrix W = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    gru::Vector x = {1.0, 2.0, 3.0};
    auto result = gru::matmul(W, x);

    ASSERT_EQ(result.size(), 2);
    ASSERT_NEAR(result[0], 14.0, 1e-6);  // 1*1 + 2*2 + 3*3
    ASSERT_NEAR(result[1], 32.0, 1e-6);  // 4*1 + 5*2 + 6*3
}

TEST(zeros_vector) {
    auto v = gru::zeros(5);
    ASSERT_EQ(v.size(), 5);
    for (const auto& val : v) {
        ASSERT_NEAR(val, 0.0, 1e-6);
    }
}

TEST(ones_vector) {
    auto v = gru::ones(4);
    ASSERT_EQ(v.size(), 4);
    for (const auto& val : v) {
        ASSERT_NEAR(val, 1.0, 1e-6);
    }
}

// Tests for GRU
TEST(gru_construction) {
    gru::GRU gru_cell(5, 10);
    ASSERT_EQ(gru_cell.input_size(), 5);
    ASSERT_EQ(gru_cell.hidden_size(), 10);
}

TEST(gru_forward_dimensions) {
    gru::GRU gru_cell(3, 5);
    gru::Vector input = {1.0, 2.0, 3.0};
    auto output = gru_cell.forward(input);

    ASSERT_EQ(output.size(), 5);
}

TEST(gru_reset_state) {
    gru::GRU gru_cell(2, 3);
    gru::Vector input = {1.0, 2.0};

    // Process input
    (void)gru_cell.forward(input);  // Explicitly discard return value

    // Reset
    gru_cell.reset_state();

    const auto& state = gru_cell.get_hidden_state();
    ASSERT_EQ(state.size(), 3);
    for (const auto& val : state) {
        ASSERT_NEAR(val, 0.0, 1e-6);
    }
}

TEST(gru_sequence_processing) {
    gru::GRU gru_cell(2, 3);

    std::vector<gru::Vector> sequence = {
        {1.0, 2.0},
        {3.0, 4.0},
        {5.0, 6.0}
    };

    // Get all outputs
    auto all_outputs = gru_cell.forward_sequence(sequence, true);
    ASSERT_EQ(all_outputs.size(), 3);

    // Reset and get only last output
    gru_cell.reset_state();
    auto last_output = gru_cell.forward_sequence(sequence, false);
    ASSERT_EQ(last_output.size(), 1);
    ASSERT_EQ(last_output[0].size(), 3);
}

TEST(gru_invalid_input) {
    gru::GRU gru_cell(3, 5);
    gru::Vector wrong_input = {1.0, 2.0};  // Wrong size

    bool exception_thrown = false;
    try {
        (void)gru_cell.forward(wrong_input);  // Explicitly discard return value
    } catch (const std::invalid_argument&) {
        exception_thrown = true;
    }

    ASSERT_TRUE(exception_thrown);
}

// Main function
int main() {
    std::cout << "\n=== Running GRU Test Suite ===\n\n";

    // Tests run via static initialization

    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Total tests: " << test_count << "\n";
    std::cout << "Passed: " << test_passed << "\n";
    std::cout << "Failed: " << (test_count - test_passed) << "\n";

    if (test_passed == test_count) {
        std::cout << "\n✓ All tests passed!\n";
        return 0;
    } else {
        std::cout << "\n✗ Some tests failed!\n";
        return 1;
    }
}
