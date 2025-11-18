#pragma once

#include "oracle.hpp"
#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace compiler_tester {

/// Statistics from a testing session
struct TestingStatistics {
    std::size_t total_programs_tested{0};
    std::size_t total_inconsistencies_found{0};
    std::size_t inconsistencies_by_type[5]{0}; // Indexed by InconsistencyType
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::map<std::string, std::size_t> compiler_issues; // compiler_version -> count

    [[nodiscard]] auto duration() const -> std::chrono::seconds {
        return std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    }

    [[nodiscard]] auto inconsistency_rate() const -> double {
        return total_programs_tested > 0
            ? static_cast<double>(total_inconsistencies_found) / total_programs_tested
            : 0.0;
    }
};

/// Report format options
enum class ReportFormat {
    Text,       // Plain text
    Markdown,   // GitHub Markdown
    JSON,       // JSON format
    HTML,       // HTML report
    XML         // XML format
};

/// Configuration for report generation
struct ReporterConfig {
    ReportFormat format{ReportFormat::Markdown};
    std::filesystem::path output_dir{"./reports"};
    bool include_source_code{true};
    bool include_assembly{false};
    bool generate_summary{true};
    bool group_by_compiler{true};
    std::optional<std::filesystem::path> template_file;
};

/// Bug report for compiler developers
struct BugReport {
    std::string title;
    std::string description;
    OptimizationInconsistency inconsistency;
    std::filesystem::path minimized_test_case;
    std::string reproduction_steps;
    std::vector<std::string> bisect_results;
    std::string environment_info;

    [[nodiscard]] auto to_markdown() const -> std::string;
    [[nodiscard]] auto to_json() const -> std::string;
};

/// Reporter for optimization inconsistencies
class Reporter {
public:
    explicit Reporter(ReporterConfig config = {});

    /// Add an inconsistency to the report
    void add_inconsistency(const OptimizationInconsistency& inconsistency);

    /// Generate and save the report
    [[nodiscard]] auto generate_report(const TestingStatistics& stats)
        -> std::filesystem::path;

    /// Generate a bug report for a specific inconsistency
    [[nodiscard]] auto generate_bug_report(
        const OptimizationInconsistency& inconsistency,
        const std::filesystem::path& test_case
    ) -> BugReport;

    /// Minimize a test case using C-Reduce
    [[nodiscard]] auto minimize_test_case(
        const std::filesystem::path& original_test,
        const OptimizationInconsistency& inconsistency
    ) -> std::optional<std::filesystem::path>;

    /// Run git bisect to find the introducing commit
    [[nodiscard]] auto bisect_regression(
        const std::filesystem::path& compiler_repo,
        const OptimizationInconsistency& inconsistency
    ) -> std::vector<std::string>;

    /// Export inconsistencies to JSON
    [[nodiscard]] auto export_to_json() const -> std::string;

    /// Export inconsistencies to CSV
    [[nodiscard]] auto export_to_csv() const -> std::string;

    /// Get all recorded inconsistencies
    [[nodiscard]] auto get_inconsistencies() const
        -> const std::vector<OptimizationInconsistency>&;

    /// Clear all recorded inconsistencies
    void clear();

private:
    ReporterConfig config_;
    std::vector<OptimizationInconsistency> inconsistencies_;

    [[nodiscard]] auto generate_text_report(const TestingStatistics& stats) const
        -> std::string;

    [[nodiscard]] auto generate_markdown_report(const TestingStatistics& stats) const
        -> std::string;

    [[nodiscard]] auto generate_json_report(const TestingStatistics& stats) const
        -> std::string;

    [[nodiscard]] auto generate_html_report(const TestingStatistics& stats) const
        -> std::string;

    [[nodiscard]] auto generate_xml_report(const TestingStatistics& stats) const
        -> std::string;

    [[nodiscard]] auto get_environment_info() const -> std::string;
};

/// Utility for creating reproducible bug reports
class BugReportBuilder {
public:
    explicit BugReportBuilder(const OptimizationInconsistency& inconsistency);

    auto set_test_case(const std::filesystem::path& path) -> BugReportBuilder&;
    auto set_compiler_version(std::string_view version) -> BugReportBuilder&;
    auto add_bisect_result(std::string_view commit) -> BugReportBuilder&;
    auto set_environment_info(std::string_view info) -> BugReportBuilder&;

    [[nodiscard]] auto build() -> BugReport;

private:
    OptimizationInconsistency inconsistency_;
    std::filesystem::path test_case_;
    std::string compiler_version_;
    std::vector<std::string> bisect_results_;
    std::string environment_info_;

    [[nodiscard]] auto generate_title() const -> std::string;
    [[nodiscard]] auto generate_description() const -> std::string;
    [[nodiscard]] auto generate_reproduction_steps() const -> std::string;
};

} // namespace compiler_tester
