#include <gtest/gtest.h>
#include "realm/mips.hpp"
#include <random>

using namespace realm;

class BruteForceMIPSTest : public ::testing::Test {
protected:
    void SetUp() override {
        mips_ = std::make_unique<BruteForceMIPS>(dim_);

        // Add some test documents
        for (size_t i = 0; i < 5; ++i) {
            DenseVector<Float> vec(dim_);
            for (size_t j = 0; j < dim_; ++j) {
                vec[j] = static_cast<Float>(i + j);
            }
            vec.normalize();
            mips_->add_document(i, vec);
            test_vectors_.push_back(vec);
        }

        mips_->build_index();
    }

    const VectorDim dim_ = 10;
    std::unique_ptr<BruteForceMIPS> mips_;
    std::vector<DenseVector<Float>> test_vectors_;
};

TEST_F(BruteForceMIPSTest, Size) {
    EXPECT_EQ(mips_->size(), 5);
}

TEST_F(BruteForceMIPSTest, SearchTopK) {
    DenseVector<Float> query = test_vectors_[0];
    auto results = mips_->search(query, 3);

    EXPECT_EQ(results.size(), 3);

    // First result should be the query itself (highest similarity)
    EXPECT_EQ(results[0].doc_id, 0);
    EXPECT_NEAR(results[0].score, 1.0f, 1e-5);

    // Results should be sorted by score (descending)
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_GE(results[i-1].score, results[i].score);
    }
}

TEST_F(BruteForceMIPSTest, SearchAllDocuments) {
    DenseVector<Float> query = test_vectors_[2];
    auto results = mips_->search(query, 10);

    // Should return all 5 documents
    EXPECT_EQ(results.size(), 5);
}

TEST_F(BruteForceMIPSTest, DimensionMismatch) {
    DenseVector<Float> wrong_dim(5);
    EXPECT_THROW(mips_->add_document(10, wrong_dim), std::invalid_argument);

    DenseVector<Float> wrong_query(5);
    EXPECT_THROW(mips_->search(wrong_query, 1), std::invalid_argument);
}

class AsymmetricLSHTest : public ::testing::Test {
protected:
    void SetUp() override {
        AsymmetricLSH::Params params;
        params.num_hash_tables = 5;
        params.hash_size = 32;

        alsh_ = std::make_unique<AsymmetricLSH>(dim_, params);

        // Add test documents
        std::random_device rd;
        std::mt19937 gen(42);  // Fixed seed for reproducibility
        std::normal_distribution<Float> dist(0.0f, 1.0f);

        for (size_t i = 0; i < 100; ++i) {
            DenseVector<Float> vec(dim_);
            for (size_t j = 0; j < dim_; ++j) {
                vec[j] = dist(gen);
            }
            vec.normalize();
            alsh_->add_document(i, vec);

            if (i < 5) {
                test_vectors_.push_back(vec);
            }
        }

        alsh_->build_index();
    }

    const VectorDim dim_ = 50;
    std::unique_ptr<AsymmetricLSH> alsh_;
    std::vector<DenseVector<Float>> test_vectors_;
};

TEST_F(AsymmetricLSHTest, Size) {
    EXPECT_EQ(alsh_->size(), 100);
}

TEST_F(AsymmetricLSHTest, SearchReturnsResults) {
    DenseVector<Float> query = test_vectors_[0];
    auto results = alsh_->search(query, 10);

    EXPECT_GT(results.size(), 0);
    EXPECT_LE(results.size(), 10);
}

TEST_F(AsymmetricLSHTest, RequiresBuildIndex) {
    AsymmetricLSH::Params params;
    AsymmetricLSH alsh_unbuild(dim_, params);

    DenseVector<Float> vec(dim_, 0.5f);
    alsh_unbuild.add_document(0, vec);

    DenseVector<Float> query(dim_, 0.5f);
    EXPECT_THROW(alsh_unbuild.search(query, 1), std::runtime_error);
}

TEST_F(AsymmetricLSHTest, ResultsSorted) {
    DenseVector<Float> query = test_vectors_[0];
    auto results = alsh_->search(query, 10);

    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_GE(results[i-1].score, results[i].score);
    }
}

TEST(MIPSResultTest, Comparison) {
    MIPSResult r1{0, 0.9f};
    MIPSResult r2{1, 0.7f};

    EXPECT_LT(r2, r1);  // r2 has lower score
}
