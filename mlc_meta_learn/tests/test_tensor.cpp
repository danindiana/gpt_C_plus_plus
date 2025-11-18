#include "mlc/tensor.hpp"
#include <gtest/gtest.h>

using namespace mlc;

TEST(TensorTest, DefaultConstructor) {
    Tensor t;
    EXPECT_EQ(t.size(), 0);
    EXPECT_EQ(t.ndim(), 0);
}

TEST(TensorTest, ShapeConstructor) {
    Tensor t({2, 3}, 1.0);

    EXPECT_EQ(t.shape().size(), 2);
    EXPECT_EQ(t.shape()[0], 2);
    EXPECT_EQ(t.shape()[1], 3);
    EXPECT_EQ(t.size(), 6);
    EXPECT_DOUBLE_EQ(t[0], 1.0);
}

TEST(TensorTest, DataConstructor) {
    std::vector<Float> data = {1.0, 2.0, 3.0, 4.0};
    Tensor t({2, 2}, data);

    EXPECT_EQ(t.size(), 4);
    EXPECT_DOUBLE_EQ(t[0], 1.0);
    EXPECT_DOUBLE_EQ(t[3], 4.0);
}

TEST(TensorTest, ElementAccess) {
    Tensor t({2, 2}, 0.0);
    t[0] = 1.0;
    t[1] = 2.0;

    EXPECT_DOUBLE_EQ(t[0], 1.0);
    EXPECT_DOUBLE_EQ(t[1], 2.0);
}

TEST(TensorTest, MultiDimensionalAccess) {
    Tensor t({2, 3}, 0.0);
    t.at(0, 0) = 1.0;
    t.at(1, 2) = 5.0;

    EXPECT_DOUBLE_EQ(t.at(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(t.at(1, 2), 5.0);
}

TEST(TensorTest, Addition) {
    Tensor t1({2, 2}, 1.0);
    Tensor t2({2, 2}, 2.0);

    auto result = t1 + t2;

    EXPECT_DOUBLE_EQ(result[0], 3.0);
    EXPECT_DOUBLE_EQ(result[3], 3.0);
}

TEST(TensorTest, Subtraction) {
    Tensor t1({2, 2}, 3.0);
    Tensor t2({2, 2}, 1.0);

    auto result = t1 - t2;

    EXPECT_DOUBLE_EQ(result[0], 2.0);
}

TEST(TensorTest, ScalarMultiplication) {
    Tensor t({2, 2}, 2.0);
    auto result = t * 3.0;

    EXPECT_DOUBLE_EQ(result[0], 6.0);
}

TEST(TensorTest, Fill) {
    Tensor t({3, 3}, 0.0);
    t.fill(5.0);

    EXPECT_DOUBLE_EQ(t[0], 5.0);
    EXPECT_DOUBLE_EQ(t[8], 5.0);
}

TEST(TensorTest, Zero) {
    Tensor t({2, 2}, 1.0);
    t.zero();

    EXPECT_DOUBLE_EQ(t[0], 0.0);
    EXPECT_DOUBLE_EQ(t[3], 0.0);
}

TEST(TensorTest, Sum) {
    Tensor t({2, 2}, 2.0);
    EXPECT_DOUBLE_EQ(t.sum(), 8.0);
}

TEST(TensorTest, Mean) {
    Tensor t({2, 2}, 4.0);
    EXPECT_DOUBLE_EQ(t.mean(), 4.0);
}

TEST(TensorTest, MaxMin) {
    std::vector<Float> data = {1.0, 5.0, 3.0, 2.0};
    Tensor t({2, 2}, data);

    EXPECT_DOUBLE_EQ(t.max(), 5.0);
    EXPECT_DOUBLE_EQ(t.min(), 1.0);
}

TEST(TensorTest, Reshape) {
    Tensor t({2, 3}, 1.0);
    auto reshaped = t.reshape({3, 2});

    EXPECT_EQ(reshaped.shape()[0], 3);
    EXPECT_EQ(reshaped.shape()[1], 2);
    EXPECT_EQ(reshaped.size(), 6);
}

TEST(ActivationTest, Sigmoid) {
    using namespace activation;

    EXPECT_DOUBLE_EQ(sigmoid(0.0), 0.5);
    EXPECT_GT(sigmoid(1.0), 0.5);
    EXPECT_LT(sigmoid(-1.0), 0.5);
}

TEST(ActivationTest, Tanh) {
    using namespace activation;

    EXPECT_DOUBLE_EQ(tanh_activation(0.0), 0.0);
    EXPECT_GT(tanh_activation(1.0), 0.0);
    EXPECT_LT(tanh_activation(-1.0), 0.0);
}

TEST(ActivationTest, ReLU) {
    using namespace activation;

    EXPECT_DOUBLE_EQ(relu(0.0), 0.0);
    EXPECT_DOUBLE_EQ(relu(5.0), 5.0);
    EXPECT_DOUBLE_EQ(relu(-5.0), 0.0);
}

TEST(ActivationTest, TensorActivations) {
    using namespace activation;

    Tensor t({2, 2}, 1.0);
    auto sig = sigmoid(t);
    auto tan = tanh(t);
    auto rel = relu(t);

    EXPECT_GT(sig[0], 0.5);
    EXPECT_GT(tan[0], 0.0);
    EXPECT_DOUBLE_EQ(rel[0], 1.0);
}
