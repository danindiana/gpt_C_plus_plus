#pragma once

#include "compiler_interface.hpp"
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace compiler_tester {

/// Types of optimization inconsistencies
enum class InconsistencyType {
    BinarySizeIncrease,      // Refined program produces larger binary
    DCERegression,           // Dead code elimination is worse
    ValueRangeRegression,    // Value range analysis is worse
    PerformanceRegression,   // Runtime performance is worse
    AssemblyDifference       // Assembly code has unexpected differences
};

/// Represents a detected optimization inconsistency
struct OptimizationInconsistency {
    InconsistencyType type;
    std::filesystem::path original_program;
    std::filesystem::path refined_program;
    CompilerType compiler;
    std::string compiler_version;
    std::string description;
    double severity{0.0}; // 0.0 to 1.0

    // Type-specific data
    std::variant<
        std::monostate,
        std::size_t,                    // For binary size difference
        std::string,                    // For assembly differences
        std::pair<double, double>       // For performance metrics
    > details;

    [[nodiscard]] auto to_string() const -> std::string;
};

/// Configuration for oracle checks
struct OracleConfig {
    // Binary size oracle
    bool enable_binary_size_check{true};
    double binary_size_threshold{0.05}; // 5% increase threshold

    // DCE oracle
    bool enable_dce_check{true};
    std::vector<std::string> dce_markers;

    // Value range oracle
    bool enable_value_range_check{true};

    // Performance oracle
    bool enable_performance_check{false};
    std::size_t performance_iterations{10};
};

/// Base class for optimization oracles
class Oracle {
public:
    virtual ~Oracle() = default;

    /// Check for optimization inconsistencies
    [[nodiscard]] virtual auto check(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::optional<OptimizationInconsistency> = 0;

    /// Get oracle name
    [[nodiscard]] virtual auto name() const noexcept -> std::string_view = 0;
};

/// Oracle that checks for binary size increases
class BinarySizeOracle final : public Oracle {
public:
    [[nodiscard]] auto check(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::optional<OptimizationInconsistency> override;

    [[nodiscard]] auto name() const noexcept -> std::string_view override {
        return "BinarySizeOracle";
    }

private:
    [[nodiscard]] static auto calculate_size_increase(
        std::size_t original_size,
        std::size_t refined_size
    ) -> double;
};

/// Oracle that checks for Dead Code Elimination regressions
class DCEOracle final : public Oracle {
public:
    [[nodiscard]] auto check(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::optional<OptimizationInconsistency> override;

    [[nodiscard]] auto name() const noexcept -> std::string_view override {
        return "DCEOracle";
    }

private:
    [[nodiscard]] static auto count_dce_markers(
        const std::filesystem::path& binary,
        const std::vector<std::string>& markers
    ) -> std::size_t;
};

/// Oracle that checks for Value Range Analysis regressions
class ValueRangeOracle final : public Oracle {
public:
    [[nodiscard]] auto check(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::optional<OptimizationInconsistency> override;

    [[nodiscard]] auto name() const noexcept -> std::string_view override {
        return "ValueRangeOracle";
    }

private:
    [[nodiscard]] static auto analyze_assembly_for_ranges(
        const std::filesystem::path& assembly_file
    ) -> std::map<std::string, ValueRange>;
};

/// Oracle that compares assembly outputs
class AssemblyOracle final : public Oracle {
public:
    [[nodiscard]] auto check(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::optional<OptimizationInconsistency> override;

    [[nodiscard]] auto name() const noexcept -> std::string_view override {
        return "AssemblyOracle";
    }

private:
    [[nodiscard]] static auto compare_assembly(
        const AssemblyCode& original,
        const AssemblyCode& refined
    ) -> std::optional<std::string>;
};

/// Composite oracle that runs multiple checks
class CompositeOracle final : public Oracle {
public:
    explicit CompositeOracle(std::vector<std::unique_ptr<Oracle>> oracles);

    [[nodiscard]] auto check(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::optional<OptimizationInconsistency> override;

    [[nodiscard]] auto name() const noexcept -> std::string_view override {
        return "CompositeOracle";
    }

    /// Check all oracles and return all detected inconsistencies
    [[nodiscard]] auto check_all(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::vector<OptimizationInconsistency>;

private:
    std::vector<std::unique_ptr<Oracle>> oracles_;
};

/// Factory for creating oracles
class OracleFactory {
public:
    [[nodiscard]] static auto create_binary_size_oracle()
        -> std::unique_ptr<Oracle>;

    [[nodiscard]] static auto create_dce_oracle()
        -> std::unique_ptr<Oracle>;

    [[nodiscard]] static auto create_value_range_oracle()
        -> std::unique_ptr<Oracle>;

    [[nodiscard]] static auto create_assembly_oracle()
        -> std::unique_ptr<Oracle>;

    [[nodiscard]] static auto create_composite_oracle()
        -> std::unique_ptr<CompositeOracle>;

    [[nodiscard]] static auto create_default_oracle()
        -> std::unique_ptr<Oracle>;
};

} // namespace compiler_tester
