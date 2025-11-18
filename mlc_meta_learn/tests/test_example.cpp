#include "mlc/example.hpp"
#include <gtest/gtest.h>

using namespace mlc;

TEST(ExampleTest, DefaultConstructor) {
    Example ex;
    EXPECT_TRUE(ex.instruction().empty());
    EXPECT_TRUE(ex.output().empty());
    EXPECT_FALSE(ex.is_valid());
}

TEST(ExampleTest, ParameterizedConstructor) {
    Example ex("jump", {"circle", "square"});

    EXPECT_EQ(ex.instruction(), "jump");
    EXPECT_EQ(ex.output().size(), 2);
    EXPECT_EQ(ex.output()[0], "circle");
    EXPECT_EQ(ex.output()[1], "square");
    EXPECT_TRUE(ex.is_valid());
}

TEST(ExampleTest, Setters) {
    Example ex;

    ex.set_instruction("skip");
    ex.set_output({"triangle"});

    EXPECT_EQ(ex.instruction(), "skip");
    EXPECT_EQ(ex.output().size(), 1);
    EXPECT_EQ(ex.output()[0], "triangle");
    EXPECT_TRUE(ex.is_valid());
}

TEST(ExampleTest, Validation) {
    Example valid("jump", {"circle"});
    EXPECT_TRUE(valid.is_valid());

    Example invalid1("jump", {});
    EXPECT_FALSE(invalid1.is_valid());

    Example invalid2("", {"circle"});
    EXPECT_FALSE(invalid2.is_valid());
}

TEST(ExampleTest, OutputSize) {
    Example ex("test", {"a", "b", "c"});
    EXPECT_EQ(ex.output_size(), 3);
}

TEST(ExampleTest, Comparison) {
    Example ex1("jump", {"circle"});
    Example ex2("jump", {"circle"});
    Example ex3("skip", {"square"});

    EXPECT_EQ(ex1, ex2);
    EXPECT_NE(ex1, ex3);
}

TEST(ExampleTest, CopySemantics) {
    Example original("jump", {"circle"});
    Example copy = original;

    EXPECT_EQ(original, copy);
    EXPECT_EQ(copy.instruction(), "jump");
    EXPECT_EQ(copy.output()[0], "circle");
}

TEST(ExampleTest, MoveSemantics) {
    Example original("jump", {"circle"});
    Example moved = std::move(original);

    EXPECT_EQ(moved.instruction(), "jump");
    EXPECT_EQ(moved.output()[0], "circle");
}
