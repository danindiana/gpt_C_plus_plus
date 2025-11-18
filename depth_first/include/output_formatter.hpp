#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <format>

namespace dfs_search {

namespace fs = std::filesystem;

/**
 * @brief Terminal color codes for colored output
 */
namespace colors {
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BOLD = "\033[1m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN = "\033[36m";
    constexpr const char* WHITE = "\033[37m";
    constexpr const char* BRIGHT_GREEN = "\033[92m";
    constexpr const char* BRIGHT_YELLOW = "\033[93m";
    constexpr const char* BRIGHT_BLUE = "\033[94m";
}

/**
 * @brief Search result entry
 */
struct SearchResult {
    fs::path file_path;
    std::string filename;
    size_t file_size{0};
    std::string file_type;
    std::chrono::system_clock::time_point modified_time;
    int depth{0};
    std::string content_match; // For content search
    size_t line_number{0};     // For content search
};

/**
 * @brief Statistics for search operations
 */
struct SearchStatistics {
    size_t files_found{0};
    size_t directories_scanned{0};
    size_t errors_encountered{0};
    std::chrono::milliseconds duration{0};
    size_t bytes_searched{0};
    size_t threads_used{1};
};

/**
 * @brief Output format types
 */
enum class OutputFormat {
    PLAIN,      // Plain text
    COLORED,    // Colored terminal output
    JSON,       // JSON format
    XML,        // XML format
    CSV         // CSV format
};

/**
 * @brief Base output formatter interface
 */
class OutputFormatter {
public:
    virtual ~OutputFormatter() = default;

    virtual void print_header(const std::string& pattern, const fs::path& root, int depth) = 0;
    virtual void print_result(const SearchResult& result) = 0;
    virtual void print_statistics(const SearchStatistics& stats) = 0;
    virtual void print_footer() = 0;
};

/**
 * @brief Plain text formatter
 */
class PlainFormatter : public OutputFormatter {
public:
    void print_header(const std::string& pattern, const fs::path& root, int depth) override {
        std::cout << "Searching for '" << pattern << "' in '" << root.string()
                  << "' (depth: " << depth << ")\n\n";
    }

    void print_result(const SearchResult& result) override {
        std::cout << "Found: " << result.file_path.string() << "\n";
    }

    void print_statistics(const SearchStatistics& stats) override {
        std::cout << "\n=== Statistics ===\n";
        std::cout << "Files found: " << stats.files_found << "\n";
        std::cout << "Directories scanned: " << stats.directories_scanned << "\n";
        std::cout << "Errors: " << stats.errors_encountered << "\n";
        std::cout << "Duration: " << stats.duration.count() << " ms\n";
    }

    void print_footer() override {}
};

/**
 * @brief Colored terminal formatter
 */
class ColoredFormatter : public OutputFormatter {
public:
    void print_header(const std::string& pattern, const fs::path& root, int depth) override {
        std::cout << colors::BOLD << colors::CYAN << "🔍 Searching for '"
                  << colors::YELLOW << pattern << colors::CYAN << "' in '"
                  << colors::BLUE << root.string() << colors::CYAN
                  << "' (depth: " << depth << ")" << colors::RESET << "\n\n";
    }

    void print_result(const SearchResult& result) override {
        std::cout << colors::BRIGHT_GREEN << "✓ " << colors::RESET
                  << colors::WHITE << result.file_path.string() << colors::RESET;

        if (result.file_size > 0) {
            std::cout << " " << colors::YELLOW << "(" << format_size(result.file_size)
                      << ")" << colors::RESET;
        }

        std::cout << "\n";
    }

    void print_statistics(const SearchStatistics& stats) override {
        std::cout << "\n" << colors::BOLD << colors::CYAN
                  << "═══════════════════════════════════════" << colors::RESET << "\n";
        std::cout << colors::BOLD << "📊 Search Statistics" << colors::RESET << "\n";
        std::cout << colors::CYAN
                  << "═══════════════════════════════════════" << colors::RESET << "\n";

        std::cout << colors::GREEN << "  Files found: " << colors::RESET
                  << colors::BOLD << stats.files_found << colors::RESET << "\n";
        std::cout << colors::BLUE << "  Directories scanned: " << colors::RESET
                  << stats.directories_scanned << "\n";
        std::cout << colors::YELLOW << "  Errors: " << colors::RESET
                  << stats.errors_encountered << "\n";
        std::cout << colors::MAGENTA << "  Duration: " << colors::RESET
                  << stats.duration.count() << " ms\n";

        if (stats.threads_used > 1) {
            std::cout << colors::CYAN << "  Threads used: " << colors::RESET
                      << stats.threads_used << "\n";
        }

        std::cout << colors::CYAN
                  << "═══════════════════════════════════════" << colors::RESET << "\n";
    }

    void print_footer() override {}

private:
    std::string format_size(size_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unit < 4) {
            size /= 1024.0;
            unit++;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
        return oss.str();
    }
};

/**
 * @brief JSON formatter
 */
class JsonFormatter : public OutputFormatter {
public:
    void print_header(const std::string& pattern, const fs::path& root, int depth) override {
        std::cout << "{\n";
        std::cout << "  \"search_parameters\": {\n";
        std::cout << "    \"pattern\": " << quote(pattern) << ",\n";
        std::cout << "    \"root_directory\": " << quote(root.string()) << ",\n";
        std::cout << "    \"max_depth\": " << depth << "\n";
        std::cout << "  },\n";
        std::cout << "  \"results\": [\n";
        first_result_ = true;
    }

    void print_result(const SearchResult& result) override {
        if (!first_result_) {
            std::cout << ",\n";
        }
        first_result_ = false;

        std::cout << "    {\n";
        std::cout << "      \"path\": " << quote(result.file_path.string()) << ",\n";
        std::cout << "      \"filename\": " << quote(result.filename) << ",\n";
        std::cout << "      \"size\": " << result.file_size << ",\n";
        std::cout << "      \"type\": " << quote(result.file_type) << ",\n";
        std::cout << "      \"depth\": " << result.depth << "\n";
        std::cout << "    }";
    }

    void print_statistics(const SearchStatistics& stats) override {
        std::cout << "\n  ],\n";
        std::cout << "  \"statistics\": {\n";
        std::cout << "    \"files_found\": " << stats.files_found << ",\n";
        std::cout << "    \"directories_scanned\": " << stats.directories_scanned << ",\n";
        std::cout << "    \"errors\": " << stats.errors_encountered << ",\n";
        std::cout << "    \"duration_ms\": " << stats.duration.count() << ",\n";
        std::cout << "    \"threads_used\": " << stats.threads_used << "\n";
        std::cout << "  }\n";
    }

    void print_footer() override {
        std::cout << "}\n";
    }

private:
    bool first_result_{true};

    std::string quote(const std::string& str) {
        std::ostringstream oss;
        oss << "\"";
        for (char c : str) {
            if (c == '"' || c == '\\') {
                oss << '\\';
            }
            oss << c;
        }
        oss << "\"";
        return oss.str();
    }
};

/**
 * @brief XML formatter
 */
class XmlFormatter : public OutputFormatter {
public:
    void print_header(const std::string& pattern, const fs::path& root, int depth) override {
        std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        std::cout << "<search>\n";
        std::cout << "  <parameters>\n";
        std::cout << "    <pattern>" << escape_xml(pattern) << "</pattern>\n";
        std::cout << "    <root_directory>" << escape_xml(root.string()) << "</root_directory>\n";
        std::cout << "    <max_depth>" << depth << "</max_depth>\n";
        std::cout << "  </parameters>\n";
        std::cout << "  <results>\n";
    }

    void print_result(const SearchResult& result) override {
        std::cout << "    <file>\n";
        std::cout << "      <path>" << escape_xml(result.file_path.string()) << "</path>\n";
        std::cout << "      <filename>" << escape_xml(result.filename) << "</filename>\n";
        std::cout << "      <size>" << result.file_size << "</size>\n";
        std::cout << "      <type>" << escape_xml(result.file_type) << "</type>\n";
        std::cout << "      <depth>" << result.depth << "</depth>\n";
        std::cout << "    </file>\n";
    }

    void print_statistics(const SearchStatistics& stats) override {
        std::cout << "  </results>\n";
        std::cout << "  <statistics>\n";
        std::cout << "    <files_found>" << stats.files_found << "</files_found>\n";
        std::cout << "    <directories_scanned>" << stats.directories_scanned << "</directories_scanned>\n";
        std::cout << "    <errors>" << stats.errors_encountered << "</errors>\n";
        std::cout << "    <duration_ms>" << stats.duration.count() << "</duration_ms>\n";
        std::cout << "    <threads_used>" << stats.threads_used << "</threads_used>\n";
        std::cout << "  </statistics>\n";
    }

    void print_footer() override {
        std::cout << "</search>\n";
    }

private:
    std::string escape_xml(const std::string& str) {
        std::ostringstream oss;
        for (char c : str) {
            switch (c) {
                case '&': oss << "&amp;"; break;
                case '<': oss << "&lt;"; break;
                case '>': oss << "&gt;"; break;
                case '"': oss << "&quot;"; break;
                case '\'': oss << "&apos;"; break;
                default: oss << c; break;
            }
        }
        return oss.str();
    }
};

/**
 * @brief CSV formatter
 */
class CsvFormatter : public OutputFormatter {
public:
    void print_header(const std::string& pattern, const fs::path& root, int depth) override {
        std::cout << "# Search Pattern: " << pattern << "\n";
        std::cout << "# Root Directory: " << root.string() << "\n";
        std::cout << "# Max Depth: " << depth << "\n";
        std::cout << "Path,Filename,Size,Type,Depth\n";
    }

    void print_result(const SearchResult& result) override {
        std::cout << quote_csv(result.file_path.string()) << ","
                  << quote_csv(result.filename) << ","
                  << result.file_size << ","
                  << quote_csv(result.file_type) << ","
                  << result.depth << "\n";
    }

    void print_statistics(const SearchStatistics& stats) override {
        std::cout << "\n# Statistics\n";
        std::cout << "# Files Found: " << stats.files_found << "\n";
        std::cout << "# Directories Scanned: " << stats.directories_scanned << "\n";
        std::cout << "# Errors: " << stats.errors_encountered << "\n";
        std::cout << "# Duration (ms): " << stats.duration.count() << "\n";
        std::cout << "# Threads Used: " << stats.threads_used << "\n";
    }

    void print_footer() override {}

private:
    std::string quote_csv(const std::string& str) {
        if (str.find(',') != std::string::npos || str.find('"') != std::string::npos) {
            std::ostringstream oss;
            oss << '"';
            for (char c : str) {
                if (c == '"') oss << '"';
                oss << c;
            }
            oss << '"';
            return oss.str();
        }
        return str;
    }
};

/**
 * @brief Factory for creating formatters
 */
class FormatterFactory {
public:
    static std::unique_ptr<OutputFormatter> create(OutputFormat format) {
        switch (format) {
            case OutputFormat::PLAIN:
                return std::make_unique<PlainFormatter>();
            case OutputFormat::COLORED:
                return std::make_unique<ColoredFormatter>();
            case OutputFormat::JSON:
                return std::make_unique<JsonFormatter>();
            case OutputFormat::XML:
                return std::make_unique<XmlFormatter>();
            case OutputFormat::CSV:
                return std::make_unique<CsvFormatter>();
            default:
                return std::make_unique<PlainFormatter>();
        }
    }
};

} // namespace dfs_search
