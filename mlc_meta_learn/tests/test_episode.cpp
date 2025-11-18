#include "mlc/episode.hpp"
#include <gtest/gtest.h>

using namespace mlc;

TEST(EpisodeTest, DefaultConstructor) {
    Episode ep;
    EXPECT_TRUE(ep.study_examples().empty());
    EXPECT_FALSE(ep.is_valid());
}

TEST(EpisodeTest, ParameterizedConstructor) {
    std::vector<Example> study = {
        Example("jump", {"circle"}),
        Example("skip", {"square"})
    };
    Example query("run", {"triangle"});

    Episode ep(study, query);

    EXPECT_EQ(ep.num_study_examples(), 2);
    EXPECT_EQ(ep.query_example().instruction(), "run");
    EXPECT_TRUE(ep.is_valid());
}

TEST(EpisodeTest, AddStudyExample) {
    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.add_study_example(Example("skip", {"square"}));

    EXPECT_EQ(ep.num_study_examples(), 2);
}

TEST(EpisodeTest, SetQueryExample) {
    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.set_query_example(Example("skip", {"square"}));

    EXPECT_EQ(ep.query_example().instruction(), "skip");
    EXPECT_TRUE(ep.is_valid());
}

TEST(EpisodeTest, ClearStudyExamples) {
    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.add_study_example(Example("skip", {"square"}));

    EXPECT_EQ(ep.num_study_examples(), 2);

    ep.clear_study_examples();
    EXPECT_EQ(ep.num_study_examples(), 0);
}

TEST(EpisodeTest, TotalExamples) {
    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.add_study_example(Example("skip", {"square"}));
    ep.set_query_example(Example("run", {"triangle"}));

    EXPECT_EQ(ep.total_examples(), 3);  // 2 study + 1 query
}

TEST(EpisodeTest, Validation) {
    Episode valid_ep;
    valid_ep.add_study_example(Example("jump", {"circle"}));
    valid_ep.set_query_example(Example("skip", {"square"}));
    EXPECT_TRUE(valid_ep.is_valid());

    Episode invalid_ep1;
    // No study examples, invalid query
    EXPECT_FALSE(invalid_ep1.is_valid());

    Episode invalid_ep2;
    invalid_ep2.add_study_example(Example("jump", {}));  // Invalid example
    invalid_ep2.set_query_example(Example("skip", {"square"}));
    EXPECT_FALSE(invalid_ep2.is_valid());
}

TEST(EpisodeTest, AllExamples) {
    Episode ep;
    ep.add_study_example(Example("jump", {"circle"}));
    ep.add_study_example(Example("skip", {"square"}));
    ep.set_query_example(Example("run", {"triangle"}));

    auto all = ep.all_examples();
    EXPECT_EQ(all.size(), 3);
}

TEST(EpisodeTest, Comparison) {
    std::vector<Example> study = {Example("jump", {"circle"})};
    Example query("skip", {"square"});

    Episode ep1(study, query);
    Episode ep2(study, query);
    Episode ep3(study, Example("run", {"triangle"}));

    EXPECT_EQ(ep1, ep2);
    EXPECT_NE(ep1, ep3);
}
