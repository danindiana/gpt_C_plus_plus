#pragma once

#include "types.hpp"
#include "example.hpp"
#include "episode.hpp"
#include <random>
#include <vector>
#include <string>
#include <algorithm>

namespace mlc::utils {

/**
 * @brief Random episode generator for training
 *
 * Generates episodes with varying complexity for meta-learning
 */
class EpisodeGenerator {
public:
    explicit EpisodeGenerator(unsigned int seed = 42)
        : rng_(seed) {
        initialize_primitives();
    }

    /**
     * @brief Generate a random training episode
     *
     * @param num_study Number of study examples
     * @param complexity Task complexity level (1-5)
     * @return Generated episode
     */
    [[nodiscard]] Episode generate_episode(
        std::size_t num_study = 5,
        int complexity = 1
    ) {
        Episode episode;

        // Generate study examples
        for (std::size_t i = 0; i < num_study; ++i) {
            episode.add_study_example(generate_example(complexity));
        }

        // Generate query example
        episode.set_query_example(generate_example(complexity));

        return episode;
    }

    /**
     * @brief Generate a compositional episode
     *
     * Creates an episode where the query requires composing learned primitives
     */
    [[nodiscard]] Episode generate_compositional_episode(std::size_t num_study = 5) {
        Episode episode;

        // Study examples teach individual primitives
        for (const auto& [inst, out] : primitive_mappings_) {
            Example ex(inst, {out});
            episode.add_study_example(ex);
            if (episode.num_study_examples() >= num_study) break;
        }

        // Query requires composition
        auto comp_inst = generate_compositional_instruction();
        auto comp_out = evaluate_compositional(comp_inst);

        episode.set_query_example(Example(comp_inst, comp_out));

        return episode;
    }

    /**
     * @brief Generate a batch of episodes
     */
    [[nodiscard]] std::vector<Episode> generate_batch(
        std::size_t batch_size,
        std::size_t num_study = 5,
        int complexity = 1
    ) {
        std::vector<Episode> batch;
        batch.reserve(batch_size);

        for (std::size_t i = 0; i < batch_size; ++i) {
            batch.push_back(generate_episode(num_study, complexity));
        }

        return batch;
    }

private:
    std::mt19937 rng_;

    // Primitive actions and objects
    std::vector<std::string> actions_ = {
        "jump", "skip", "tiptoe", "run", "walk", "hop", "leap"
    };

    std::vector<std::string> objects_ = {
        "circle", "square", "triangle", "diamond", "star", "hexagon"
    };

    std::vector<std::string> modifiers_ = {
        "twice", "thrice", "once", "left", "right", "around"
    };

    // Primitive instruction -> output mappings
    std::vector<std::pair<std::string, std::string>> primitive_mappings_;

    void initialize_primitives() {
        primitive_mappings_ = {
            {"jump", "circle"},
            {"skip", "square"},
            {"tiptoe", "triangle"},
            {"run", "diamond"},
            {"walk", "star"}
        };
    }

    [[nodiscard]] Example generate_example(int complexity) {
        std::string instruction;
        OutputSequence output;

        if (complexity == 1) {
            // Simple primitive mapping
            auto& [inst, out] = primitive_mappings_[
                std::uniform_int_distribution<>(0, primitive_mappings_.size() - 1)(rng_)
            ];
            instruction = inst;
            output = {out};
        } else if (complexity == 2) {
            // Action with modifier
            auto action = actions_[
                std::uniform_int_distribution<>(0, actions_.size() - 1)(rng_)
            ];
            auto modifier = modifiers_[
                std::uniform_int_distribution<>(0, modifiers_.size() - 1)(rng_)
            ];
            instruction = action + " " + modifier;

            auto obj = objects_[
                std::uniform_int_distribution<>(0, objects_.size() - 1)(rng_)
            ];
            output = {obj};

            if (modifier == "twice") {
                output.push_back(obj);
            } else if (modifier == "thrice") {
                output.push_back(obj);
                output.push_back(obj);
            }
        } else {
            // Complex composition
            instruction = generate_compositional_instruction();
            output = evaluate_compositional(instruction);
        }

        return Example(instruction, output);
    }

    [[nodiscard]] std::string generate_compositional_instruction() {
        std::uniform_int_distribution<> action_dist(0, actions_.size() - 1);
        std::uniform_int_distribution<> modifier_dist(0, modifiers_.size() - 1);

        auto action1 = actions_[action_dist(rng_)];
        auto action2 = actions_[action_dist(rng_)];
        auto modifier = modifiers_[modifier_dist(rng_)];

        return action1 + " and " + action2 + " " + modifier;
    }

    [[nodiscard]] OutputSequence evaluate_compositional(const std::string& instruction) {
        // Simple evaluation - in practice, this would be more sophisticated
        OutputSequence result;

        std::uniform_int_distribution<> obj_dist(0, objects_.size() - 1);

        // Parse instruction and generate output
        if (instruction.find("twice") != std::string::npos) {
            auto obj = objects_[obj_dist(rng_)];
            result = {obj, obj};
        } else if (instruction.find("thrice") != std::string::npos) {
            auto obj = objects_[obj_dist(rng_)];
            result = {obj, obj, obj};
        } else if (instruction.find("and") != std::string::npos) {
            result = {objects_[obj_dist(rng_)], objects_[obj_dist(rng_)]};
        } else {
            result = {objects_[obj_dist(rng_)]};
        }

        return result;
    }
};

/**
 * @brief Utility functions for training and evaluation
 */
namespace training {

/**
 * @brief Split episodes into train/validation/test sets
 */
inline auto split_episodes(
    const std::vector<Episode>& episodes,
    Float train_ratio = 0.7,
    Float val_ratio = 0.15
) {
    std::size_t total = episodes.size();
    std::size_t train_size = static_cast<std::size_t>(total * train_ratio);
    std::size_t val_size = static_cast<std::size_t>(total * val_ratio);

    struct Split {
        std::vector<Episode> train;
        std::vector<Episode> validation;
        std::vector<Episode> test;
    };

    Split split;
    split.train.assign(episodes.begin(), episodes.begin() + train_size);
    split.validation.assign(
        episodes.begin() + train_size,
        episodes.begin() + train_size + val_size
    );
    split.test.assign(episodes.begin() + train_size + val_size, episodes.end());

    return split;
}

/**
 * @brief Shuffle episodes for training
 */
inline void shuffle_episodes(std::vector<Episode>& episodes, unsigned int seed = 42) {
    std::mt19937 rng(seed);
    std::shuffle(episodes.begin(), episodes.end(), rng);
}

} // namespace training

} // namespace mlc::utils
