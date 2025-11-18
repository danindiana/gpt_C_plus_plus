#include <gtest/gtest.h>
#include "realm/types.hpp"

using namespace realm;

class DenseVectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        vec1_ = DenseVector<Float>(3);
        vec1_[0] = 1.0f;
        vec1_[1] = 2.0f;
        vec1_[2] = 3.0f;

        vec2_ = DenseVector<Float>(3);
        vec2_[0] = 4.0f;
        vec2_[1] = 5.0f;
        vec2_[2] = 6.0f;
    }

    DenseVector<Float> vec1_;
    DenseVector<Float> vec2_;
};

TEST_F(DenseVectorTest, Construction) {
    DenseVector<Float> vec(5, 1.0f);
    EXPECT_EQ(vec.dimension(), 5);
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_FLOAT_EQ(vec[i], 1.0f);
    }
}

TEST_F(DenseVectorTest, InitializerList) {
    DenseVector<Float> vec = {1.0f, 2.0f, 3.0f};
    EXPECT_EQ(vec.dimension(), 3);
    EXPECT_FLOAT_EQ(vec[0], 1.0f);
    EXPECT_FLOAT_EQ(vec[1], 2.0f);
    EXPECT_FLOAT_EQ(vec[2], 3.0f);
}

TEST_F(DenseVectorTest, DotProduct) {
    Float result = vec1_.dot(vec2_);
    EXPECT_FLOAT_EQ(result, 32.0f);  // 1*4 + 2*5 + 3*6 = 32
}

TEST_F(DenseVectorTest, Norm) {
    DenseVector<Float> vec = {3.0f, 4.0f};
    Float norm = vec.norm();
    EXPECT_FLOAT_EQ(norm, 5.0f);  // sqrt(9 + 16) = 5
}

TEST_F(DenseVectorTest, Normalize) {
    DenseVector<Float> vec = {3.0f, 4.0f};
    vec.normalize();
    EXPECT_NEAR(vec.norm(), 1.0f, 1e-6);
    EXPECT_NEAR(vec[0], 0.6f, 1e-6);
    EXPECT_NEAR(vec[1], 0.8f, 1e-6);
}

TEST_F(DenseVectorTest, DimensionMismatch) {
    DenseVector<Float> vec3(4);
    EXPECT_THROW(vec1_.dot(vec3), std::invalid_argument);
}

TEST(DocumentTest, Construction) {
    Document doc(1, "This is a test document");
    EXPECT_EQ(doc.id, 1);
    EXPECT_EQ(doc.content, "This is a test document");
    EXPECT_EQ(doc.tokens.size(), 5);
    EXPECT_EQ(doc.tokens[0], "This");
    EXPECT_EQ(doc.tokens[4], "document");
}

TEST(QueryTest, Construction) {
    Query query(42, "What is machine learning?");
    EXPECT_EQ(query.id, 42);
    EXPECT_EQ(query.content, "What is machine learning?");
    EXPECT_EQ(query.tokens.size(), 4);
}

TEST(HyperParamsTest, DefaultValues) {
    HyperParams params;
    EXPECT_EQ(params.num_epochs, 100);
    EXPECT_EQ(params.batch_size, 32);
    EXPECT_FLOAT_EQ(params.learning_rate, 0.001f);
    EXPECT_EQ(params.document_vector_size, 768);
}

TEST(TrainingStatsTest, Update) {
    TrainingStats stats;
    stats.update(1.5f, 2.5f, 10);
    EXPECT_FLOAT_EQ(stats.mlm_loss, 1.5f);
    EXPECT_FLOAT_EQ(stats.retrieval_loss, 2.5f);
    EXPECT_FLOAT_EQ(stats.total_loss, 4.0f);
    EXPECT_EQ(stats.epoch, 10);
}
