#pragma once

#include "example.hpp"
#include "types.hpp"
#include <vector>
#include <optional>
#include <algorithm>
#include <ranges>

namespace mlc {

/**
 * @brief Represents a training or evaluation episode
 *
 * An Episode contains study examples (for learning) and a query example (for testing).
 * This structure enables meta-learning by providing few-shot learning scenarios.
 */
class Episode {
public:
    // Constructors
    Episode() = default;

    explicit Episode(std::vector<Example> study, Example query)
        : study_examples_(std::move(study))
        , query_example_(std::move(query)) {}

    // Copy and move semantics
    Episode(const Episode&) = default;
    Episode(Episode&&) noexcept = default;
    Episode& operator=(const Episode&) = default;
    Episode& operator=(Episode&&) noexcept = default;

    ~Episode() = default;

    // Accessors
    [[nodiscard]] const std::vector<Example>& study_examples() const noexcept {
        return study_examples_;
    }

    [[nodiscard]] const Example& query_example() const noexcept {
        return query_example_;
    }

    [[nodiscard]] std::vector<Example>& study_examples() noexcept {
        return study_examples_;
    }

    [[nodiscard]] Example& query_example() noexcept {
        return query_example_;
    }

    // Setters
    void set_study_examples(std::vector<Example> examples) {
        study_examples_ = std::move(examples);
    }

    void set_query_example(Example example) {
        query_example_ = std::move(example);
    }

    // Modifiers
    void add_study_example(Example example) {
        study_examples_.push_back(std::move(example));
    }

    void clear_study_examples() noexcept {
        study_examples_.clear();
    }

    // Utility methods
    [[nodiscard]] std::size_t num_study_examples() const noexcept {
        return study_examples_.size();
    }

    [[nodiscard]] bool is_valid() const noexcept {
        if (study_examples_.empty() || !query_example_.is_valid()) {
            return false;
        }
        return std::ranges::all_of(study_examples_,
            [](const auto& ex) { return ex.is_valid(); });
    }

    [[nodiscard]] std::size_t total_examples() const noexcept {
        return study_examples_.size() + 1; // +1 for query
    }

    // Get a view of all examples (study + query)
    [[nodiscard]] std::vector<std::reference_wrapper<const Example>> all_examples() const {
        std::vector<std::reference_wrapper<const Example>> all;
        all.reserve(total_examples());
        for (const auto& ex : study_examples_) {
            all.emplace_back(std::cref(ex));
        }
        all.emplace_back(std::cref(query_example_));
        return all;
    }

    // Comparison operators
    [[nodiscard]] bool operator==(const Episode& other) const noexcept {
        return study_examples_ == other.study_examples_ &&
               query_example_ == other.query_example_;
    }

    [[nodiscard]] bool operator!=(const Episode& other) const noexcept {
        return !(*this == other);
    }

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const Episode& ep) {
        os << "Episode{\n  study_examples: [\n";
        for (const auto& ex : ep.study_examples_) {
            os << "    " << ex << "\n";
        }
        os << "  ],\n  query: " << ep.query_example_ << "\n}";
        return os;
    }

private:
    std::vector<Example> study_examples_;
    Example query_example_;
};

} // namespace mlc
