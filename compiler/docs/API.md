# API Documentation

## Overview

The Compiler Optimization Tester provides a modern C++23 API for detecting compiler optimization inconsistencies. All components are in the `compiler_tester` namespace.

## Core Modules

### Program Generator

Generate random test programs using industry-standard tools.

#### ProgramGenerator Interface

```cpp
class ProgramGenerator {
public:
    virtual ~ProgramGenerator() = default;

    // Generate a single test program
    [[nodiscard]] virtual auto generate(const GeneratorConfig& config)
        -> std::optional<GeneratedProgram> = 0;

    // Generate multiple test programs
    [[nodiscard]] virtual auto generate_batch(
        const GeneratorConfig& config,
        std::size_t count
    ) -> std::vector<GeneratedProgram> = 0;

    // Get generator name
    [[nodiscard]] virtual auto name() const noexcept
        -> std::string_view = 0;

    // Check availability
    [[nodiscard]] virtual auto is_available() const noexcept
        -> bool = 0;
};
```

#### GeneratorConfig

```cpp
struct GeneratorConfig {
    std::filesystem::path output_dir{"./generated"};  // Output directory
    std::size_t seed{0};                              // Random seed
    bool deterministic{true};                         // Use deterministic generation
    std::size_t max_depth{10};                        // Maximum nesting depth
    std::size_t max_functions{20};                    // Maximum number of functions

    // Optional: custom tool paths
    std::optional<std::filesystem::path> csmith_path;
    std::optional<std::filesystem::path> yarpgen_path;
};
```

#### GeneratedProgram

```cpp
struct GeneratedProgram {
    std::filesystem::path file_path;  // Path to generated file
    std::string source_code;          // Complete source code
    std::size_t seed;                 // Seed used for generation
    std::string generator_name;       // Name of generator used

    [[nodiscard]] auto is_valid() const noexcept -> bool;
};
```

#### GeneratorFactory

```cpp
class GeneratorFactory {
public:
    // Create Csmith generator
    [[nodiscard]] static auto create_csmith(
        std::optional<std::filesystem::path> binary_path = std::nullopt
    ) -> std::unique_ptr<ProgramGenerator>;

    // Create YarpGen generator
    [[nodiscard]] static auto create_yarpgen(
        std::optional<std::filesystem::path> binary_path = std::nullopt
    ) -> std::unique_ptr<ProgramGenerator>;

    // Create default available generator
    [[nodiscard]] static auto create_default()
        -> std::unique_ptr<ProgramGenerator>;
};
```

#### Example Usage

```cpp
#include "compiler_tester/program_generator.hpp"

using namespace compiler_tester;

// Create generator
auto generator = GeneratorFactory::create_csmith();

// Configure generation
GeneratorConfig config{
    .output_dir = "./test_programs",
    .seed = 42,
    .deterministic = true,
    .max_functions = 15
};

// Generate single program
if (auto program = generator->generate(config)) {
    std::cout << "Generated: " << program->file_path << "\n";
}

// Generate batch
auto programs = generator->generate_batch(config, 10);
std::cout << "Generated " << programs.size() << " programs\n";
```

### Information Injector

Inject optimization hints into programs.

#### InformationInjector

```cpp
class InformationInjector {
public:
    explicit InformationInjector(InjectorConfig config = {});

    // Inject dead code information
    [[nodiscard]] auto inject_dead_code_info(
        std::string_view source_code,
        const std::vector<DeadCodeInfo>& dead_code_info
    ) -> std::string;

    // Inject value range information
    [[nodiscard]] auto inject_value_range_info(
        std::string_view source_code,
        const std::vector<ValueRangeInfo>& value_range_info
    ) -> std::string;

    // Inject all information
    [[nodiscard]] auto inject_all(
        const std::filesystem::path& original_file,
        const ProgramInformation& info
    ) -> InjectionResult;

    // Derive information from program
    [[nodiscard]] static auto derive_information(
        const std::filesystem::path& program_path
    ) -> std::optional<ProgramInformation>;
};
```

#### InjectorConfig

```cpp
struct InjectorConfig {
    bool use_builtin_unreachable{true};  // Use __builtin_unreachable()
    bool use_builtin_assume{false};       // Use __builtin_assume()
    bool use_cpp23_assume{false};         // Use [[assume()]]
    bool preserve_formatting{true};       // Preserve original formatting
    bool add_comments{true};              // Add explanatory comments
};
```

#### ProgramInformation

```cpp
struct ProgramInformation {
    std::vector<DeadCodeInfo> dead_code_locations;
    std::vector<ValueRangeInfo> value_ranges;

    [[nodiscard]] auto has_information() const noexcept -> bool;
};

struct DeadCodeInfo {
    std::size_t line_number;
    std::size_t column;
    std::string function_name;
    std::string condition;
};

struct ValueRangeInfo {
    std::string variable_name;
    std::size_t line_number;
    ValueRange range;
};

struct ValueRange {
    long long min_value;
    long long max_value;

    [[nodiscard]] auto is_valid() const noexcept -> bool;
    [[nodiscard]] auto is_constant() const noexcept -> bool;
};
```

#### Example Usage

```cpp
#include "compiler_tester/information_injector.hpp"

using namespace compiler_tester;

// Create injector
InjectorConfig config{
    .use_builtin_unreachable = true,
    .add_comments = true
};

InformationInjector injector(config);

// Create program information
ProgramInformation info;

info.dead_code_locations.push_back({
    .line_number = 42,
    .column = 5,
    .function_name = "main",
    .condition = "x != 0"
});

info.value_ranges.push_back({
    .variable_name = "count",
    .line_number = 50,
    .range = {.min_value = 0, .max_value = 100}
});

// Inject information
auto result = injector.inject_all("program.c", info);

if (result.is_valid()) {
    std::cout << "Created refined program: "
              << result.refined_file << "\n";
    std::cout << "Injections: " << result.injections_count << "\n";
}
```

### Compiler Interface

Abstract interface for multiple compilers.

#### CompilerInterface

```cpp
class CompilerInterface {
public:
    virtual ~CompilerInterface() = default;

    // Compile a source file
    [[nodiscard]] virtual auto compile(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> CompilationResult = 0;

    // Get assembly output
    [[nodiscard]] virtual auto get_assembly(
        const std::filesystem::path& source_file,
        const CompilerConfig& config
    ) -> std::optional<AssemblyCode> = 0;

    // Get compiler version
    [[nodiscard]] virtual auto get_version() const -> std::string = 0;

    // Get compiler type
    [[nodiscard]] virtual auto get_type() const noexcept
        -> CompilerType = 0;

    // Check if compiler is available
    [[nodiscard]] virtual auto is_available() const noexcept -> bool = 0;

    // Get supported optimization levels
    [[nodiscard]] virtual auto supported_optimization_levels() const
        -> std::vector<OptimizationLevel> = 0;
};
```

#### CompilerConfig

```cpp
struct CompilerConfig {
    std::filesystem::path compiler_path;
    OptimizationLevel optimization_level{OptimizationLevel::O3};
    std::vector<std::string> extra_flags;
    std::filesystem::path output_dir{"./build"};
    bool generate_assembly{false};
    bool generate_llvm_ir{false};
    bool enable_warnings{true};
    std::optional<std::chrono::seconds> timeout;

    [[nodiscard]] auto get_optimization_flag() const -> std::string;
};

enum class OptimizationLevel {
    O0,    // No optimization
    O1,    // Basic optimization
    O2,    // Moderate optimization
    O3,    // Aggressive optimization
    Os,    // Optimize for size
    Ofast  // Aggressive, may break standards
};

enum class CompilerType {
    GCC,
    Clang,
    MSVC,
    ICC,
    Unknown
};
```

#### CompilationResult

```cpp
struct CompilationResult {
    bool success{false};
    std::filesystem::path binary_path;
    std::string stdout_output;
    std::string stderr_output;
    std::size_t binary_size{0};
    std::chrono::milliseconds compilation_time{0};
    int exit_code{0};

    [[nodiscard]] auto is_valid() const noexcept -> bool;
};
```

#### CompilerFactory

```cpp
class CompilerFactory {
public:
    // Create GCC compiler
    [[nodiscard]] static auto create_gcc(
        std::optional<std::filesystem::path> binary_path = std::nullopt
    ) -> std::unique_ptr<CompilerInterface>;

    // Create Clang compiler
    [[nodiscard]] static auto create_clang(
        std::optional<std::filesystem::path> binary_path = std::nullopt
    ) -> std::unique_ptr<CompilerInterface>;

    // Detect all available compilers
    [[nodiscard]] static auto detect_available_compilers()
        -> std::vector<std::unique_ptr<CompilerInterface>>;
};
```

#### Example Usage

```cpp
#include "compiler_tester/compiler_interface.hpp"

using namespace compiler_tester;

// Create compiler
auto compiler = CompilerFactory::create_gcc();

// Configure compilation
CompilerConfig config{
    .optimization_level = OptimizationLevel::O3,
    .output_dir = "./build",
    .enable_warnings = true
};

// Add custom flags
config.extra_flags.push_back("-march=native");
config.extra_flags.push_back("-mtune=native");

// Compile
auto result = compiler->compile("test.c", config);

if (result.success) {
    std::cout << "Binary size: " << result.binary_size << " bytes\n";
    std::cout << "Compilation time: "
              << result.compilation_time.count() << " ms\n";
}

// Get assembly
if (auto assembly = compiler->get_assembly("test.c", config)) {
    std::cout << "Assembly:\n" << assembly->content << "\n";
}
```

### Oracle System

Detect optimization inconsistencies.

#### Oracle Interface

```cpp
class Oracle {
public:
    virtual ~Oracle() = default;

    // Check for inconsistencies
    [[nodiscard]] virtual auto check(
        const CompilationResult& original_result,
        const CompilationResult& refined_result,
        const OracleConfig& config
    ) -> std::optional<OptimizationInconsistency> = 0;

    // Get oracle name
    [[nodiscard]] virtual auto name() const noexcept
        -> std::string_view = 0;
};
```

#### OracleConfig

```cpp
struct OracleConfig {
    // Binary size oracle
    bool enable_binary_size_check{true};
    double binary_size_threshold{0.05};  // 5%

    // DCE oracle
    bool enable_dce_check{true};
    std::vector<std::string> dce_markers;

    // Value range oracle
    bool enable_value_range_check{true};

    // Performance oracle
    bool enable_performance_check{false};
    std::size_t performance_iterations{10};
};
```

#### OptimizationInconsistency

```cpp
struct OptimizationInconsistency {
    InconsistencyType type;
    std::filesystem::path original_program;
    std::filesystem::path refined_program;
    CompilerType compiler;
    std::string compiler_version;
    std::string description;
    double severity{0.0};  // 0.0 to 1.0

    std::variant<
        std::monostate,
        std::size_t,
        std::string,
        std::pair<double, double>
    > details;

    [[nodiscard]] auto to_string() const -> std::string;
};

enum class InconsistencyType {
    BinarySizeIncrease,
    DCERegression,
    ValueRangeRegression,
    PerformanceRegression,
    AssemblyDifference
};
```

#### OracleFactory

```cpp
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
```

#### Example Usage

```cpp
#include "compiler_tester/oracle.hpp"

using namespace compiler_tester;

// Create oracle
auto oracle = OracleFactory::create_binary_size_oracle();

// Configure
OracleConfig config{
    .enable_binary_size_check = true,
    .binary_size_threshold = 0.05  // 5%
};

// Check for inconsistencies
if (auto inc = oracle->check(original_result, refined_result, config)) {
    std::cout << "Inconsistency detected!\n";
    std::cout << "Type: " << static_cast<int>(inc->type) << "\n";
    std::cout << "Severity: " << (inc->severity * 100) << "%\n";
    std::cout << inc->to_string() << "\n";
}

// Use composite oracle
auto composite = OracleFactory::create_composite_oracle();
auto all_issues = composite->check_all(
    original_result,
    refined_result,
    config
);

std::cout << "Found " << all_issues.size() << " issues\n";
```

### Reporter

Generate reports and bug reports.

#### Reporter

```cpp
class Reporter {
public:
    explicit Reporter(ReporterConfig config = {});

    // Add inconsistency
    void add_inconsistency(const OptimizationInconsistency& inc);

    // Generate report
    [[nodiscard]] auto generate_report(const TestingStatistics& stats)
        -> std::filesystem::path;

    // Generate bug report
    [[nodiscard]] auto generate_bug_report(
        const OptimizationInconsistency& inconsistency,
        const std::filesystem::path& test_case
    ) -> BugReport;

    // Minimize test case
    [[nodiscard]] auto minimize_test_case(
        const std::filesystem::path& original_test,
        const OptimizationInconsistency& inconsistency
    ) -> std::optional<std::filesystem::path>;

    // Run git bisect
    [[nodiscard]] auto bisect_regression(
        const std::filesystem::path& compiler_repo,
        const OptimizationInconsistency& inconsistency
    ) -> std::vector<std::string>;

    // Export functions
    [[nodiscard]] auto export_to_json() const -> std::string;
    [[nodiscard]] auto export_to_csv() const -> std::string;

    // Get all inconsistencies
    [[nodiscard]] auto get_inconsistencies() const
        -> const std::vector<OptimizationInconsistency>&;

    // Clear all inconsistencies
    void clear();
};
```

#### ReporterConfig

```cpp
struct ReporterConfig {
    ReportFormat format{ReportFormat::Markdown};
    std::filesystem::path output_dir{"./reports"};
    bool include_source_code{true};
    bool include_assembly{false};
    bool generate_summary{true};
    bool group_by_compiler{true};
    std::optional<std::filesystem::path> template_file;
};

enum class ReportFormat {
    Text,
    Markdown,
    JSON,
    HTML,
    XML
};
```

#### TestingStatistics

```cpp
struct TestingStatistics {
    std::size_t total_programs_tested{0};
    std::size_t total_inconsistencies_found{0};
    std::size_t inconsistencies_by_type[5]{0};
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::map<std::string, std::size_t> compiler_issues;

    [[nodiscard]] auto duration() const -> std::chrono::seconds;
    [[nodiscard]] auto inconsistency_rate() const -> double;
};
```

#### Example Usage

```cpp
#include "compiler_tester/reporter.hpp"

using namespace compiler_tester;

// Create reporter
ReporterConfig config{
    .format = ReportFormat::Markdown,
    .output_dir = "./reports",
    .include_source_code = true
};

Reporter reporter(config);

// Add inconsistencies
for (const auto& inc : inconsistencies) {
    reporter.add_inconsistency(inc);
}

// Generate report
TestingStatistics stats{
    .total_programs_tested = 100,
    .total_inconsistencies_found = 5
};

stats.start_time = std::chrono::system_clock::now();
// ... run tests ...
stats.end_time = std::chrono::system_clock::now();

auto report_path = reporter.generate_report(stats);
std::cout << "Report saved to: " << report_path << "\n";

// Export to JSON
auto json = reporter.export_to_json();
std::ofstream("results.json") << json;
```

## Complete Example

```cpp
#include "compiler_tester/program_generator.hpp"
#include "compiler_tester/information_injector.hpp"
#include "compiler_tester/compiler_interface.hpp"
#include "compiler_tester/oracle.hpp"
#include "compiler_tester/reporter.hpp"

#include <iostream>

using namespace compiler_tester;

int main() {
    // Setup components
    auto generator = GeneratorFactory::create_default();
    InformationInjector injector;
    auto compiler = CompilerFactory::create_gcc();
    auto oracle = OracleFactory::create_default_oracle();
    Reporter reporter;

    // Configuration
    GeneratorConfig gen_config{.seed = 42};
    CompilerConfig comp_config{.optimization_level = OptimizationLevel::O3};
    OracleConfig oracle_config;

    // Statistics
    TestingStatistics stats;
    stats.start_time = std::chrono::system_clock::now();

    // Generate and test programs
    for (std::size_t i = 0; i < 10; ++i) {
        auto program = generator->generate(gen_config);
        if (!program) continue;

        // Inject information
        ProgramInformation info;
        auto refined = injector.inject_all(program->file_path, info);

        // Compile both versions
        auto original_result = compiler->compile(program->file_path, comp_config);
        auto refined_result = compiler->compile(refined.refined_file, comp_config);

        stats.total_programs_tested++;

        // Check for inconsistencies
        if (auto inc = oracle->check(original_result, refined_result, oracle_config)) {
            std::cout << "Found inconsistency in program " << i << "\n";
            reporter.add_inconsistency(*inc);
            stats.total_inconsistencies_found++;
        }
    }

    stats.end_time = std::chrono::system_clock::now();

    // Generate report
    auto report = reporter.generate_report(stats);
    std::cout << "Report saved to: " << report << "\n";

    return stats.total_inconsistencies_found > 0 ? 2 : 0;
}
```

---

For more information, see:
- [Architecture Documentation](ARCHITECTURE.md)
- [Usage Guide](USAGE.md)
- [Main README](../README.md)
