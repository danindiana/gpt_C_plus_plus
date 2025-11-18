#include "compiler_tester/reporter.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace compiler_tester {

namespace {

auto get_timestamp() -> std::string {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

auto escape_json(std::string_view str) -> std::string {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

} // anonymous namespace

// BugReport implementations
auto BugReport::to_markdown() const -> std::string {
    std::ostringstream oss;

    oss << "# " << title << "\n\n";
    oss << "## Description\n\n" << description << "\n\n";

    oss << "## Environment\n\n";
    oss << "```\n" << environment_info << "\n```\n\n";

    oss << "## Reproduction Steps\n\n" << reproduction_steps << "\n\n";

    if (!minimized_test_case.empty()) {
        oss << "## Test Case\n\n";
        oss << "File: `" << minimized_test_case.string() << "`\n\n";
    }

    if (!bisect_results.empty()) {
        oss << "## Bisection Results\n\n";
        for (const auto& result : bisect_results) {
            oss << "- " << result << "\n";
        }
        oss << "\n";
    }

    oss << "## Inconsistency Details\n\n";
    oss << inconsistency.to_string() << "\n";

    return oss.str();
}

auto BugReport::to_json() const -> std::string {
    std::ostringstream oss;

    oss << "{\n";
    oss << "  \"title\": \"" << escape_json(title) << "\",\n";
    oss << "  \"description\": \"" << escape_json(description) << "\",\n";
    oss << "  \"environment\": \"" << escape_json(environment_info) << "\",\n";
    oss << "  \"test_case\": \"" << escape_json(minimized_test_case.string()) << "\",\n";
    oss << "  \"reproduction_steps\": \"" << escape_json(reproduction_steps) << "\"\n";
    oss << "}";

    return oss.str();
}

// Reporter implementation
Reporter::Reporter(ReporterConfig config)
    : config_(std::move(config)) {
    std::filesystem::create_directories(config_.output_dir);
}

void Reporter::add_inconsistency(const OptimizationInconsistency& inconsistency) {
    inconsistencies_.push_back(inconsistency);
}

auto Reporter::generate_report(const TestingStatistics& stats) -> std::filesystem::path {
    std::string content;

    switch (config_.format) {
        case ReportFormat::Text:
            content = generate_text_report(stats);
            break;
        case ReportFormat::Markdown:
            content = generate_markdown_report(stats);
            break;
        case ReportFormat::JSON:
            content = generate_json_report(stats);
            break;
        case ReportFormat::HTML:
            content = generate_html_report(stats);
            break;
        case ReportFormat::XML:
            content = generate_xml_report(stats);
            break;
    }

    auto report_file = config_.output_dir / ("report_" + get_timestamp() + ".md");

    std::ofstream file(report_file);
    file << content;

    return report_file;
}

auto Reporter::generate_bug_report(
    const OptimizationInconsistency& inconsistency,
    const std::filesystem::path& test_case
) -> BugReport {

    BugReportBuilder builder(inconsistency);
    builder.set_test_case(test_case)
           .set_compiler_version(inconsistency.compiler_version)
           .set_environment_info(get_environment_info());

    return builder.build();
}

auto Reporter::minimize_test_case(
    const std::filesystem::path& /*original_test*/,
    const OptimizationInconsistency& /*inconsistency*/
) -> std::optional<std::filesystem::path> {
    // C-Reduce integration would be implemented here
    return std::nullopt;
}

auto Reporter::bisect_regression(
    const std::filesystem::path& /*compiler_repo*/,
    const OptimizationInconsistency& /*inconsistency*/
) -> std::vector<std::string> {
    // git bisect automation would be implemented here
    return {};
}

auto Reporter::export_to_json() const -> std::string {
    std::ostringstream oss;

    oss << "[\n";
    for (std::size_t i = 0; i < inconsistencies_.size(); ++i) {
        const auto& inc = inconsistencies_[i];
        oss << "  {\n";
        oss << "    \"type\": " << static_cast<int>(inc.type) << ",\n";
        oss << "    \"description\": \"" << escape_json(inc.description) << "\",\n";
        oss << "    \"severity\": " << inc.severity << "\n";
        oss << "  }";
        if (i < inconsistencies_.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }
    oss << "]\n";

    return oss.str();
}

auto Reporter::export_to_csv() const -> std::string {
    std::ostringstream oss;

    oss << "Type,Compiler,Version,Severity,Description\n";

    for (const auto& inc : inconsistencies_) {
        oss << static_cast<int>(inc.type) << ","
            << static_cast<int>(inc.compiler) << ","
            << inc.compiler_version << ","
            << inc.severity << ","
            << "\"" << inc.description << "\"\n";
    }

    return oss.str();
}

auto Reporter::get_inconsistencies() const
    -> const std::vector<OptimizationInconsistency>& {
    return inconsistencies_;
}

void Reporter::clear() {
    inconsistencies_.clear();
}

auto Reporter::generate_text_report(const TestingStatistics& stats) const -> std::string {
    std::ostringstream oss;

    oss << "Compiler Optimization Testing Report\n";
    oss << "====================================\n\n";
    oss << "Generated: " << get_timestamp() << "\n\n";

    oss << "Summary\n";
    oss << "-------\n";
    oss << "Total programs tested: " << stats.total_programs_tested << "\n";
    oss << "Inconsistencies found: " << stats.total_inconsistencies_found << "\n";
    oss << "Inconsistency rate: " << (stats.inconsistency_rate() * 100.0) << "%\n";
    oss << "Duration: " << stats.duration().count() << " seconds\n\n";

    oss << "Inconsistencies\n";
    oss << "---------------\n";
    for (const auto& inc : inconsistencies_) {
        oss << inc.to_string() << "\n\n";
    }

    return oss.str();
}

auto Reporter::generate_markdown_report(const TestingStatistics& stats) const -> std::string {
    std::ostringstream oss;

    oss << "# Compiler Optimization Testing Report\n\n";
    oss << "*Generated: " << get_timestamp() << "*\n\n";

    oss << "## Summary\n\n";
    oss << "- **Total Programs Tested**: " << stats.total_programs_tested << "\n";
    oss << "- **Inconsistencies Found**: " << stats.total_inconsistencies_found << "\n";
    oss << "- **Inconsistency Rate**: " << (stats.inconsistency_rate() * 100.0) << "%\n";
    oss << "- **Duration**: " << stats.duration().count() << " seconds\n\n";

    oss << "## Detected Inconsistencies\n\n";

    for (std::size_t i = 0; i < inconsistencies_.size(); ++i) {
        const auto& inc = inconsistencies_[i];
        oss << "### Issue #" << (i + 1) << "\n\n";
        oss << "```\n" << inc.to_string() << "\n```\n\n";
    }

    return oss.str();
}

auto Reporter::generate_json_report(const TestingStatistics& /*stats*/) const -> std::string {
    return export_to_json();
}

auto Reporter::generate_html_report(const TestingStatistics& stats) const -> std::string {
    // HTML generation would be more elaborate
    return "<html><body><h1>Report</h1><p>Total: " +
           std::to_string(stats.total_programs_tested) + "</p></body></html>";
}

auto Reporter::generate_xml_report(const TestingStatistics& /*stats*/) const -> std::string {
    return "<?xml version=\"1.0\"?>\n<report></report>";
}

auto Reporter::get_environment_info() const -> std::string {
    return "OS: Linux\nArchitecture: x86_64";
}

// BugReportBuilder implementation
BugReportBuilder::BugReportBuilder(const OptimizationInconsistency& inconsistency)
    : inconsistency_(inconsistency) {}

auto BugReportBuilder::set_test_case(const std::filesystem::path& path) -> BugReportBuilder& {
    test_case_ = path;
    return *this;
}

auto BugReportBuilder::set_compiler_version(std::string_view version) -> BugReportBuilder& {
    compiler_version_ = version;
    return *this;
}

auto BugReportBuilder::add_bisect_result(std::string_view commit) -> BugReportBuilder& {
    bisect_results_.push_back(std::string(commit));
    return *this;
}

auto BugReportBuilder::set_environment_info(std::string_view info) -> BugReportBuilder& {
    environment_info_ = info;
    return *this;
}

auto BugReportBuilder::build() -> BugReport {
    return BugReport{
        .title = generate_title(),
        .description = generate_description(),
        .inconsistency = inconsistency_,
        .minimized_test_case = test_case_,
        .reproduction_steps = generate_reproduction_steps(),
        .bisect_results = bisect_results_,
        .environment_info = environment_info_
    };
}

auto BugReportBuilder::generate_title() const -> std::string {
    return "Optimization inconsistency: refined program produces larger binary";
}

auto BugReportBuilder::generate_description() const -> std::string {
    return inconsistency_.description;
}

auto BugReportBuilder::generate_reproduction_steps() const -> std::string {
    return "1. Compile the original program\n2. Compile the refined program\n3. Compare outputs";
}

} // namespace compiler_tester
