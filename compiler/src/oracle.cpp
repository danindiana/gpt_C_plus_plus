#include "compiler_tester/oracle.hpp"
#include <sstream>
#include <cmath>

namespace compiler_tester {

// OptimizationInconsistency implementation
auto OptimizationInconsistency::to_string() const -> std::string {
    std::ostringstream oss;

    oss << "Optimization Inconsistency Detected\n";
    oss << "Type: ";

    switch (type) {
        case InconsistencyType::BinarySizeIncrease:
            oss << "Binary Size Increase";
            break;
        case InconsistencyType::DCERegression:
            oss << "Dead Code Elimination Regression";
            break;
        case InconsistencyType::ValueRangeRegression:
            oss << "Value Range Analysis Regression";
            break;
        case InconsistencyType::PerformanceRegression:
            oss << "Performance Regression";
            break;
        case InconsistencyType::AssemblyDifference:
            oss << "Assembly Difference";
            break;
    }

    oss << "\nCompiler: ";
    switch (compiler) {
        case CompilerType::GCC: oss << "GCC"; break;
        case CompilerType::Clang: oss << "Clang/LLVM"; break;
        case CompilerType::MSVC: oss << "MSVC"; break;
        case CompilerType::ICC: oss << "ICC"; break;
        case CompilerType::Unknown: oss << "Unknown"; break;
    }

    oss << " " << compiler_version << "\n";
    oss << "Original: " << original_program << "\n";
    oss << "Refined: " << refined_program << "\n";
    oss << "Severity: " << (severity * 100.0) << "%\n";
    oss << "Description: " << description << "\n";

    return oss.str();
}

// BinarySizeOracle implementation
auto BinarySizeOracle::check(
    const CompilationResult& original_result,
    const CompilationResult& refined_result,
    const OracleConfig& config
) -> std::optional<OptimizationInconsistency> {

    if (!config.enable_binary_size_check) {
        return std::nullopt;
    }

    if (!original_result.is_valid() || !refined_result.is_valid()) {
        return std::nullopt;
    }

    auto size_increase = calculate_size_increase(
        original_result.binary_size,
        refined_result.binary_size
    );

    if (size_increase <= config.binary_size_threshold) {
        return std::nullopt;
    }

    OptimizationInconsistency inconsistency;
    inconsistency.type = InconsistencyType::BinarySizeIncrease;
    inconsistency.original_program = original_result.binary_path;
    inconsistency.refined_program = refined_result.binary_path;
    inconsistency.severity = std::min(size_increase, 1.0);
    inconsistency.details = refined_result.binary_size - original_result.binary_size;

    std::ostringstream desc;
    desc << "Refined program binary is " << (size_increase * 100.0)
         << "% larger than original ("
         << refined_result.binary_size << " vs "
         << original_result.binary_size << " bytes)";
    inconsistency.description = desc.str();

    return inconsistency;
}

auto BinarySizeOracle::calculate_size_increase(
    std::size_t original_size,
    std::size_t refined_size
) -> double {

    if (original_size == 0) {
        return refined_size > 0 ? 1.0 : 0.0;
    }

    if (refined_size <= original_size) {
        return 0.0;
    }

    return static_cast<double>(refined_size - original_size) /
           static_cast<double>(original_size);
}

// DCEOracle implementation
auto DCEOracle::check(
    const CompilationResult& original_result,
    const CompilationResult& refined_result,
    const OracleConfig& config
) -> std::optional<OptimizationInconsistency> {

    if (!config.enable_dce_check) {
        return std::nullopt;
    }

    // DCE analysis would require examining assembly or binary
    // This is a placeholder implementation
    return std::nullopt;
}

auto DCEOracle::count_dce_markers(
    const std::filesystem::path& /*binary*/,
    const std::vector<std::string>& /*markers*/
) -> std::size_t {
    // Would parse binary/assembly for dead code markers
    return 0;
}

// ValueRangeOracle implementation
auto ValueRangeOracle::check(
    const CompilationResult& original_result,
    const CompilationResult& refined_result,
    const OracleConfig& config
) -> std::optional<OptimizationInconsistency> {

    if (!config.enable_value_range_check) {
        return std::nullopt;
    }

    // Value range analysis would require examining compiler output
    // This is a placeholder implementation
    return std::nullopt;
}

auto ValueRangeOracle::analyze_assembly_for_ranges(
    const std::filesystem::path& /*assembly_file*/
) -> std::map<std::string, ValueRange> {
    // Would parse assembly to extract value range information
    return {};
}

// AssemblyOracle implementation
auto AssemblyOracle::check(
    const CompilationResult& original_result,
    const CompilationResult& refined_result,
    const OracleConfig& /*config*/
) -> std::optional<OptimizationInconsistency> {

    if (!original_result.is_valid() || !refined_result.is_valid()) {
        return std::nullopt;
    }

    // Assembly comparison would be implemented here
    return std::nullopt;
}

auto AssemblyOracle::compare_assembly(
    const AssemblyCode& /*original*/,
    const AssemblyCode& /*refined*/
) -> std::optional<std::string> {
    // Would perform detailed assembly comparison
    return std::nullopt;
}

// CompositeOracle implementation
CompositeOracle::CompositeOracle(std::vector<std::unique_ptr<Oracle>> oracles)
    : oracles_(std::move(oracles)) {}

auto CompositeOracle::check(
    const CompilationResult& original_result,
    const CompilationResult& refined_result,
    const OracleConfig& config
) -> std::optional<OptimizationInconsistency> {

    for (const auto& oracle : oracles_) {
        if (auto inconsistency = oracle->check(original_result, refined_result, config)) {
            return inconsistency;
        }
    }

    return std::nullopt;
}

auto CompositeOracle::check_all(
    const CompilationResult& original_result,
    const CompilationResult& refined_result,
    const OracleConfig& config
) -> std::vector<OptimizationInconsistency> {

    std::vector<OptimizationInconsistency> inconsistencies;

    for (const auto& oracle : oracles_) {
        if (auto inconsistency = oracle->check(original_result, refined_result, config)) {
            inconsistencies.push_back(std::move(*inconsistency));
        }
    }

    return inconsistencies;
}

// OracleFactory implementation
auto OracleFactory::create_binary_size_oracle() -> std::unique_ptr<Oracle> {
    return std::make_unique<BinarySizeOracle>();
}

auto OracleFactory::create_dce_oracle() -> std::unique_ptr<Oracle> {
    return std::make_unique<DCEOracle>();
}

auto OracleFactory::create_value_range_oracle() -> std::unique_ptr<Oracle> {
    return std::make_unique<ValueRangeOracle>();
}

auto OracleFactory::create_assembly_oracle() -> std::unique_ptr<Oracle> {
    return std::make_unique<AssemblyOracle>();
}

auto OracleFactory::create_composite_oracle() -> std::unique_ptr<CompositeOracle> {
    std::vector<std::unique_ptr<Oracle>> oracles;
    oracles.push_back(create_binary_size_oracle());
    oracles.push_back(create_dce_oracle());
    oracles.push_back(create_value_range_oracle());
    oracles.push_back(create_assembly_oracle());

    return std::make_unique<CompositeOracle>(std::move(oracles));
}

auto OracleFactory::create_default_oracle() -> std::unique_ptr<Oracle> {
    return create_binary_size_oracle();
}

} // namespace compiler_tester
