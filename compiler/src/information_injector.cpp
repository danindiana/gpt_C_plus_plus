#include "compiler_tester/information_injector.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace compiler_tester {

namespace {

auto split_lines(std::string_view text) -> std::vector<std::string> {
    std::vector<std::string> lines;
    std::istringstream stream{std::string(text)};
    std::string line;

    while (std::getline(stream, line)) {
        lines.push_back(std::move(line));
    }

    return lines;
}

auto join_lines(const std::vector<std::string>& lines) -> std::string {
    std::ostringstream result;
    for (const auto& line : lines) {
        result << line << '\n';
    }
    return result.str();
}

} // anonymous namespace

InformationInjector::InformationInjector(InjectorConfig config)
    : config_(std::move(config)) {}

auto InformationInjector::inject_dead_code_info(
    std::string_view source_code,
    const std::vector<DeadCodeInfo>& dead_code_info
) -> std::string {

    auto lines = split_lines(source_code);

    // Sort by line number in descending order to avoid offset issues
    auto sorted_info = dead_code_info;
    std::sort(sorted_info.begin(), sorted_info.end(),
        [](const auto& a, const auto& b) {
            return a.line_number > b.line_number;
        });

    for (const auto& info : sorted_info) {
        if (info.line_number < lines.size()) {
            auto statement = generate_unreachable_statement(info.condition);

            if (config_.add_comments) {
                statement = "    // Injected dead code information\n" + statement;
            }

            lines.insert(lines.begin() + info.line_number, statement);
        }
    }

    return join_lines(lines);
}

auto InformationInjector::inject_value_range_info(
    std::string_view source_code,
    const std::vector<ValueRangeInfo>& value_range_info
) -> std::string {

    auto lines = split_lines(source_code);

    auto sorted_info = value_range_info;
    std::sort(sorted_info.begin(), sorted_info.end(),
        [](const auto& a, const auto& b) {
            return a.line_number > b.line_number;
        });

    for (const auto& info : sorted_info) {
        if (info.line_number < lines.size() && info.range.is_valid()) {
            auto assertion = generate_range_assertion(info.variable_name, info.range);

            if (config_.add_comments) {
                assertion = "    // Injected value range information\n" + assertion;
            }

            lines.insert(lines.begin() + info.line_number, assertion);
        }
    }

    return join_lines(lines);
}

auto InformationInjector::inject_all(
    const std::filesystem::path& original_file,
    const ProgramInformation& info
) -> InjectionResult {

    std::ifstream file(original_file);
    if (!file) {
        return {.success = false};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    auto source = buffer.str();

    // Inject dead code information
    if (!info.dead_code_locations.empty()) {
        source = inject_dead_code_info(source, info.dead_code_locations);
    }

    // Inject value range information
    if (!info.value_ranges.empty()) {
        source = inject_value_range_info(source, info.value_ranges);
    }

    // Create refined file path
    auto refined_path = original_file.parent_path() /
        (original_file.stem().string() + "_refined" + original_file.extension().string());

    std::ofstream out_file(refined_path);
    if (!out_file) {
        return {.success = false};
    }

    out_file << source;

    return InjectionResult{
        .refined_file = refined_path,
        .refined_code = std::move(source),
        .injections_count = info.dead_code_locations.size() + info.value_ranges.size(),
        .success = true
    };
}

auto InformationInjector::derive_information(const std::filesystem::path& /*program_path*/)
    -> std::optional<ProgramInformation> {
    // This would require runtime analysis or static analysis
    // For now, return empty information
    // In a full implementation, this would:
    // 1. Run the program with instrumentation
    // 2. Collect value ranges from execution
    // 3. Identify dead code paths
    return ProgramInformation{};
}

auto InformationInjector::generate_unreachable_statement(std::string_view condition) const
    -> std::string {

    if (config_.use_cpp23_assume) {
        return "    [[assume(" + std::string(condition) + ")]];";
    }

    if (config_.use_builtin_assume) {
        return "    __builtin_assume(" + std::string(condition) + ");";
    }

    // Default: use __builtin_unreachable()
    return "    if (!(" + std::string(condition) + ")) __builtin_unreachable();";
}

auto InformationInjector::generate_range_assertion(
    std::string_view variable,
    const ValueRange& range
) const -> std::string {

    if (range.is_constant()) {
        auto condition = std::string(variable) + " == " + std::to_string(range.min_value);
        return generate_unreachable_statement(condition);
    }

    // Generate range check: min <= var <= max
    auto condition = "(" + std::string(variable) + " >= " + std::to_string(range.min_value) +
                    " && " + std::string(variable) + " <= " + std::to_string(range.max_value) + ")";

    return generate_unreachable_statement(condition);
}

auto InformationInjector::insert_at_line(
    std::string_view source_code,
    std::size_t line_number,
    std::string_view insertion
) const -> std::string {

    auto lines = split_lines(source_code);

    if (line_number < lines.size()) {
        lines.insert(lines.begin() + line_number, std::string(insertion));
    }

    return join_lines(lines);
}

} // namespace compiler_tester
