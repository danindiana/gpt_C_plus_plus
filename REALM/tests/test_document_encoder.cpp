#include <gtest/gtest.h>
#include "realm/document_encoder.hpp"

using namespace realm;

class SimpleEncoderTest : public ::testing::Test {
protected:
    void SetUp() override {
        encoder_ = std::make_unique<SimpleEncoder>(128, 1000);
    }

    std::unique_ptr<SimpleEncoder> encoder_;
};

TEST_F(SimpleEncoderTest, EmbeddingDimension) {
    EXPECT_EQ(encoder_->embedding_dimension(), 128);
}

TEST_F(SimpleEncoderTest, EncodeDocument) {
    Document doc(1, "This is a test");
    auto vec = encoder_->encode_document(doc);

    EXPECT_EQ(vec.dimension(), 128);

    // Encoded vector should be normalized
    EXPECT_NEAR(vec.norm(), 1.0f, 1e-5);
}

TEST_F(SimpleEncoderTest, EncodeQuery) {
    Query query(1, "What is this?");
    auto vec = encoder_->encode_query(query);

    EXPECT_EQ(vec.dimension(), 128);
    EXPECT_NEAR(vec.norm(), 1.0f, 1e-5);
}

TEST_F(SimpleEncoderTest, ConsistentEncoding) {
    Document doc(1, "Same content");
    auto vec1 = encoder_->encode_document(doc);
    auto vec2 = encoder_->encode_document(doc);

    // Same document should produce same encoding
    EXPECT_FLOAT_EQ(vec1.dot(vec2), 1.0f);
}

TEST_F(SimpleEncoderTest, BatchEncoding) {
    std::vector<Document> docs = {
        {1, "First document"},
        {2, "Second document"},
        {3, "Third document"}
    };

    auto encoded = encoder_->encode_documents_batch(docs);

    EXPECT_EQ(encoded.size(), 3);
    for (const auto& vec : encoded) {
        EXPECT_EQ(vec.dimension(), 128);
        EXPECT_NEAR(vec.norm(), 1.0f, 1e-5);
    }
}

TEST_F(SimpleEncoderTest, EmptyDocument) {
    Document doc(1, "");
    auto vec = encoder_->encode_document(doc);

    EXPECT_EQ(vec.dimension(), 128);
    // Empty document should produce zero vector
    EXPECT_FLOAT_EQ(vec.norm(), 0.0f);
}

TEST(BERTEncoderTest, Construction) {
    BERTEncoder encoder("/path/to/model", 768);
    EXPECT_EQ(encoder.embedding_dimension(), 768);
}
