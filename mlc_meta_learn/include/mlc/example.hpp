#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <optional>
#include <iostream>

namespace mlc {

/**
 * @brief Represents a single training or test example
 *
 * An Example consists of an instruction and the corresponding output sequence.
 * This is the fundamental unit of data in the MLC framework.
 */
class Example {
public:
    // Constructors
    Example() = default;

    explicit Example(Instruction instr)
        : instruction_(std::move(instr)) {}

    Example(Instruction instr, OutputSequence out)
        : instruction_(std::move(instr))
        , output_(std::move(out)) {}

    // Copy and move semantics
    Example(const Example&) = default;
    Example(Example&&) noexcept = default;
    Example& operator=(const Example&) = default;
    Example& operator=(Example&&) noexcept = default;

    ~Example() = default;

    // Accessors
    [[nodiscard]] const Instruction& instruction() const noexcept {
        return instruction_;
    }

    [[nodiscard]] const OutputSequence& output() const noexcept {
        return output_;
    }

    [[nodiscard]] Instruction& instruction() noexcept {
        return instruction_;
    }

    [[nodiscard]] OutputSequence& output() noexcept {
        return output_;
    }

    // Setters
    void set_instruction(Instruction instr) {
        instruction_ = std::move(instr);
    }

    void set_output(OutputSequence out) {
        output_ = std::move(out);
    }

    // Utility methods
    [[nodiscard]] bool is_valid() const noexcept {
        return !instruction_.empty() && !output_.empty();
    }

    [[nodiscard]] std::size_t output_size() const noexcept {
        return output_.size();
    }

    // Comparison operators
    [[nodiscard]] bool operator==(const Example& other) const noexcept {
        return instruction_ == other.instruction_ && output_ == other.output_;
    }

    [[nodiscard]] bool operator!=(const Example& other) const noexcept {
        return !(*this == other);
    }

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const Example& ex) {
        os << "Example{instruction: \"" << ex.instruction_ << "\", output: [";
        for (std::size_t i = 0; i < ex.output_.size(); ++i) {
            os << "\"" << ex.output_[i] << "\"";
            if (i < ex.output_.size() - 1) os << ", ";
        }
        os << "]}";
        return os;
    }

private:
    Instruction instruction_;
    OutputSequence output_;
};

} // namespace mlc
