#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace compiler_tester {

/// Configuration for program generation
struct GeneratorConfig {
    std::filesystem::path output_dir{"./generated"};
    std::size_t seed{0};
    bool deterministic{true};
    std::size_t max_depth{10};
    std::size_t max_functions{20};

    // Tool-specific settings
    std::optional<std::filesystem::path> csmith_path;
    std::optional<std::filesystem::path> yarpgen_path;
};

/// Represents a generated test program
struct GeneratedProgram {
    std::filesystem::path file_path;
    std::string source_code;
    std::size_t seed;
    std::string generator_name;

    [[nodiscard]] auto is_valid() const noexcept -> bool {
        return std::filesystem::exists(file_path) && !source_code.empty();
    }
};

/// Interface for program generators
class ProgramGenerator {
public:
    virtual ~ProgramGenerator() = default;

    /// Generate a single test program
    [[nodiscard]] virtual auto generate(const GeneratorConfig& config)
        -> std::optional<GeneratedProgram> = 0;

    /// Generate multiple test programs
    [[nodiscard]] virtual auto generate_batch(const GeneratorConfig& config, std::size_t count)
        -> std::vector<GeneratedProgram> = 0;

    /// Get the name of this generator
    [[nodiscard]] virtual auto name() const noexcept -> std::string_view = 0;

    /// Check if the generator is available on this system
    [[nodiscard]] virtual auto is_available() const noexcept -> bool = 0;
};

/// Csmith-based program generator
class CsmithGenerator final : public ProgramGenerator {
public:
    explicit CsmithGenerator(std::filesystem::path csmith_binary = "/usr/bin/csmith");

    [[nodiscard]] auto generate(const GeneratorConfig& config)
        -> std::optional<GeneratedProgram> override;

    [[nodiscard]] auto generate_batch(const GeneratorConfig& config, std::size_t count)
        -> std::vector<GeneratedProgram> override;

    [[nodiscard]] auto name() const noexcept -> std::string_view override {
        return "Csmith";
    }

    [[nodiscard]] auto is_available() const noexcept -> bool override;

private:
    std::filesystem::path binary_path_;
};

/// YarpGen-based program generator
class YarpGenGenerator final : public ProgramGenerator {
public:
    explicit YarpGenGenerator(std::filesystem::path yarpgen_binary = "/usr/bin/yarpgen");

    [[nodiscard]] auto generate(const GeneratorConfig& config)
        -> std::optional<GeneratedProgram> override;

    [[nodiscard]] auto generate_batch(const GeneratorConfig& config, std::size_t count)
        -> std::vector<GeneratedProgram> override;

    [[nodiscard]] auto name() const noexcept -> std::string_view override {
        return "YarpGen";
    }

    [[nodiscard]] auto is_available() const noexcept -> bool override;

private:
    std::filesystem::path binary_path_;
};

/// Factory for creating program generators
class GeneratorFactory {
public:
    [[nodiscard]] static auto create_csmith(std::optional<std::filesystem::path> binary_path = std::nullopt)
        -> std::unique_ptr<ProgramGenerator>;

    [[nodiscard]] static auto create_yarpgen(std::optional<std::filesystem::path> binary_path = std::nullopt)
        -> std::unique_ptr<ProgramGenerator>;

    [[nodiscard]] static auto create_default() -> std::unique_ptr<ProgramGenerator>;
};

} // namespace compiler_tester
