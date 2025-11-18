/**
 * @file dfs_simple.cpp
 * @brief Simplified modern DFS search without external dependencies
 * @version 2.0.0
 *
 * Modernized version of the original file search programs using C++20/23 features
 * but without requiring external libraries.
 */

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <regex>
#include <chrono>
#include <format>
#include <ranges>

namespace fs = std::filesystem;

class PatternMatcher {
public:
    explicit PatternMatcher(const std::string& pattern, bool use_regex = false)
        : pattern_(pattern), use_regex_(use_regex) {
        if (use_regex_) {
            regex_ = std::regex(pattern, std::regex::ECMAScript);
        }
    }

    [[nodiscard]] bool matches(const std::string& filename) const {
        if (use_regex_) {
            return std::regex_match(filename, regex_);
        }

        // Wildcard matching
        if (pattern_ == "*") return true;

        if (pattern_.starts_with("*.")) {
            std::string ext = pattern_.substr(1);
            return filename.ends_with(ext);
        }

        if (pattern_.starts_with("*")) {
            return filename.contains(pattern_.substr(1));
        }

        if (pattern_.ends_with("*")) {
            return filename.starts_with(pattern_.substr(0, pattern_.size() - 1));
        }

        return filename == pattern_;
    }

private:
    std::string pattern_;
    bool use_regex_;
    mutable std::regex regex_;
};

class SearchStatistics {
public:
    size_t files_found = 0;
    size_t dirs_scanned = 0;
    size_t errors = 0;

    void print() const {
        std::cout << std::format("\n=== Statistics ===\n");
        std::cout << std::format("Files found: {}\n", files_found);
        std::cout << std::format("Directories scanned: {}\n", dirs_scanned);
        std::cout << std::format("Errors: {}\n", errors);
    }
};

std::vector<std::string> default_exclusions = {
    "/proc", "/sys", "/dev", "/tmp", "/run",
    ".git", ".svn", "node_modules", "__pycache__"
};

bool shouldSkip(const fs::path& path, const std::vector<std::string>& exclusions) {
    std::string pathStr = path.string();
    return std::any_of(exclusions.begin(), exclusions.end(), [&](const auto& excluded) {
        return pathStr.find(excluded) != std::string::npos;
    });
}

void dfsSearch(const fs::path& current, const PatternMatcher& matcher,
               int depth, SearchStatistics& stats,
               const std::vector<std::string>& exclusions, bool verbose) {

    if (shouldSkip(current, exclusions)) {
        if (verbose) {
            std::cout << std::format("Skipping: {}\n", current.string());
        }
        return;
    }

    try {
        if (depth >= 0) {
            stats.dirs_scanned++;

            for (const auto& entry : fs::directory_iterator(current)) {
                if (entry.is_directory()) {
                    dfsSearch(entry.path(), matcher, depth - 1, stats, exclusions, verbose);
                } else if (matcher.matches(entry.path().filename().string())) {
                    std::cout << std::format("Found: {}\n", entry.path().string());
                    stats.files_found++;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        stats.errors++;
        if (verbose) {
            std::cerr << std::format("Error: {}\n", e.what());
        }
    }
}

void printUsage(const char* prog) {
    std::cout << std::format("Usage: {} <pattern> <directory> [max_depth] [options]\n\n", prog);
    std::cout << "Options:\n";
    std::cout << "  -r      Use regex pattern matching\n";
    std::cout << "  -v      Verbose output\n";
    std::cout << "  -h      Show this help\n\n";
    std::cout << "Examples:\n";
    std::cout << std::format("  {} '*.txt' . 5\n", prog);
    std::cout << std::format("  {} 'test.*\\.cpp' /home/user 10 -r\n", prog);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    // Parse arguments
    std::string pattern = argv[1];
    std::string root_dir = argv[2];
    int max_depth = (argc >= 4) ? std::stoi(argv[3]) : 10;

    bool use_regex = false;
    bool verbose = false;

    // Parse flags
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-r") use_regex = true;
        else if (arg == "-v") verbose = true;
        else if (arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }

    // Validate directory
    fs::path root_path(root_dir);
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
        std::cerr << std::format("Error: '{}' is not a valid directory\n", root_dir);
        return 1;
    }

    // Execute search
    std::cout << std::format("Searching for '{}' in '{}' (depth: {})\n\n",
        pattern, root_dir, max_depth);

    auto start = std::chrono::steady_clock::now();

    PatternMatcher matcher(pattern, use_regex);
    SearchStatistics stats;

    dfsSearch(root_path, matcher, max_depth, stats, default_exclusions, verbose);

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (stats.files_found == 0) {
        std::cout << "No matching files found.\n";
    }

    stats.print();
    std::cout << std::format("Duration: {} ms\n", duration.count());

    return (stats.files_found > 0) ? 0 : 1;
}
