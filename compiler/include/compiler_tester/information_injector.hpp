#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace compiler_tester {

/// Represents a value range for a variable
struct ValueRange {
    long long min_value;
    long long max_value;

    [[nodiscard]] auto is_valid() const noexcept -> bool {
        return min_value <= max_value;
    }

    [[nodiscard]] auto is_constant() const noexcept -> bool {
        return min_value == max_value;
    }
};

/// Information about unreachable code locations
struct DeadCodeInfo {
    std::size_t line_number;
    std::size_t column;
    std::string function_name;
    std::string condition; // The condition that makes this code unreachable
};

/// Information about variable value ranges at specific locations
struct ValueRangeInfo {
    std::string variable_name;
    std::size_t line_number;
    ValueRange range;
};

/// All additional information that can be injected into a program
struct ProgramInformation {
    std::vector<DeadCodeInfo> dead_code_locations;
    std::vector<ValueRangeInfo> value_ranges;

    [[nodiscard]] auto has_information() const noexcept -> bool {
        return !dead_code_locations.empty() || !value_ranges.empty();
    }
};

/// Configuration for information injection
struct InjectorConfig {
    bool use_builtin_unreachable{true};
    bool use_builtin_assume{false};
    bool use_cpp23_assume{false};
    bool preserve_formatting{true};
    bool add_comments{true};
};

/// Result of information injection
struct InjectionResult {
    std::filesystem::path refined_file;
    std::string refined_code;
    std::size_t injections_count{0};
    bool success{false};

    [[nodiscard]] auto is_valid() const noexcept -> bool {
        return success && std::filesystem::exists(refined_file);
    }
};

/// Injects additional optimization information into programs
class InformationInjector {
public:
    explicit InformationInjector(InjectorConfig config = {});

    /// Inject dead code information using __builtin_unreachable()
    [[nodiscard]] auto inject_dead_code_info(
        std::string_view source_code,
        const std::vector<DeadCodeInfo>& dead_code_info
    ) -> std::string;

    /// Inject value range information
    [[nodiscard]] auto inject_value_range_info(
        std::string_view source_code,
        const std::vector<ValueRangeInfo>& value_range_info
    ) -> std::string;

    /// Inject all program information
    [[nodiscard]] auto inject_all(
        const std::filesystem::path& original_file,
        const ProgramInformation& info
    ) -> InjectionResult;

    /// Derive information from running a program
    [[nodiscard]] static auto derive_information(
        const std::filesystem::path& program_path
    ) -> std::optional<ProgramInformation>;

    /// Generate __builtin_unreachable() statement
    [[nodiscard]] auto generate_unreachable_statement(
        std::string_view condition
    ) const -> std::string;

    /// Generate value range assertion
    [[nodiscard]] auto generate_range_assertion(
        std::string_view variable,
        const ValueRange& range
    ) const -> std::string;

private:
    InjectorConfig config_;

    [[nodiscard]] auto insert_at_line(
        std::string_view source_code,
        std::size_t line_number,
        std::string_view insertion
    ) const -> std::string;
};

} // namespace compiler_tester
