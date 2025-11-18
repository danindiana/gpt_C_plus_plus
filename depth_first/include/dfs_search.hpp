#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <regex>
#include <optional>
#include <chrono>
#include <iostream>
#include <format>
#include <concepts>
#include <ranges>
#include <algorithm>
#include <system_error>
#include <mutex>
#include <atomic>

namespace dfs_search {

namespace fs = std::filesystem;
using namespace std::chrono_literals;

// C++20 Concepts for type safety
template<typename T>
concept PathLike = std::convertible_to<T, fs::path>;

template<typename T>
concept PatternMatcher = requires(T matcher, const std::string& str) {
    { matcher(str) } -> std::convertible_to<bool>;
};

// Modern pattern matching with regex support
class PatternMatcher {
public:
    enum class MatchType {
        WILDCARD,  // Simple wildcards like *.txt
        REGEX,     // Full regex support
        EXACT      // Exact match
    };

    explicit PatternMatcher(std::string pattern, MatchType type = MatchType::WILDCARD)
        : pattern_(std::move(pattern)), type_(type) {

        if (type_ == MatchType::REGEX) {
            regex_ = std::regex(pattern_, std::regex::ECMAScript | std::regex::icase);
        }
    }

    [[nodiscard]] bool matches(const std::string& filename) const {
        switch (type_) {
            case MatchType::WILDCARD:
                return matchWildcard(filename);
            case MatchType::REGEX:
                return std::regex_match(filename, regex_);
            case MatchType::EXACT:
                return filename == pattern_;
        }
        return false;
    }

private:
    [[nodiscard]] bool matchWildcard(const std::string& filename) const {
        if (pattern_ == "*") return true;

        // Handle *.ext patterns
        if (pattern_.size() > 1 && pattern_[0] == '*' && pattern_[1] == '.') {
            std::string ext = pattern_.substr(1);
            return filename.size() >= ext.size() &&
                   filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0;
        }

        // Handle *pattern patterns
        if (pattern_[0] == '*') {
            return filename.find(pattern_.substr(1)) != std::string::npos;
        }

        // Handle pattern* patterns
        if (pattern_.back() == '*') {
            return filename.starts_with(pattern_.substr(0, pattern_.size() - 1));
        }

        return filename == pattern_;
    }

    std::string pattern_;
    MatchType type_;
    mutable std::regex regex_;
};

// Search statistics
struct SearchStats {
    std::atomic<size_t> files_found{0};
    std::atomic<size_t> directories_scanned{0};
    std::atomic<size_t> errors_encountered{0};
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    void start() { start_time = std::chrono::steady_clock::now(); }
    void stop() { end_time = std::chrono::steady_clock::now(); }

    [[nodiscard]] auto duration() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    void print() const {
        std::cout << std::format("\n=== Search Statistics ===\n");
        std::cout << std::format("Files found: {}\n", files_found.load());
        std::cout << std::format("Directories scanned: {}\n", directories_scanned.load());
        std::cout << std::format("Errors encountered: {}\n", errors_encountered.load());
        std::cout << std::format("Duration: {} ms\n", duration().count());
    }
};

// Exclusion list manager
class ExclusionList {
public:
    ExclusionList() {
        // Default system paths to exclude
        excluded_paths_ = {
            "/proc", "/sys", "/dev", "/tmp", "/run",
            ".git", ".svn", ".hg", "node_modules", "__pycache__"
        };
    }

    void add(std::string path) {
        std::lock_guard lock(mutex_);
        excluded_paths_.push_back(std::move(path));
    }

    void addMultiple(const std::vector<std::string>& paths) {
        std::lock_guard lock(mutex_);
        excluded_paths_.insert(excluded_paths_.end(), paths.begin(), paths.end());
    }

    [[nodiscard]] bool shouldSkip(const fs::path& path) const {
        std::lock_guard lock(mutex_);
        std::string pathStr = path.string();

        return std::ranges::any_of(excluded_paths_, [&pathStr](const auto& excluded) {
            return pathStr.find(excluded) != std::string::npos;
        });
    }

private:
    mutable std::mutex mutex_;
    std::vector<std::string> excluded_paths_;
};

// Modern error handling
[[nodiscard]] inline bool isPermissionDenied(const std::error_code& ec) noexcept {
    return ec.value() == EACCES || ec.value() == EPERM;
}

// Search options configuration
struct SearchOptions {
    bool verbose{false};
    bool follow_symlinks{false};
    bool case_sensitive{true};
    bool stop_on_first_match{false};
    bool show_statistics{true};
    std::optional<size_t> max_results{std::nullopt};
    PatternMatcher::MatchType match_type{PatternMatcher::MatchType::WILDCARD};
};

// Base search class with modern C++20 features
class FileSearcher {
public:
    explicit FileSearcher(SearchOptions options = {})
        : options_(std::move(options)), stats_{} {}

    virtual ~FileSearcher() = default;

    // Disable copy, enable move
    FileSearcher(const FileSearcher&) = delete;
    FileSearcher& operator=(const FileSearcher&) = delete;
    FileSearcher(FileSearcher&&) = default;
    FileSearcher& operator=(FileSearcher&&) = default;

    [[nodiscard]] const SearchStats& stats() const { return stats_; }
    [[nodiscard]] const ExclusionList& exclusions() const { return exclusions_; }
    ExclusionList& exclusions() { return exclusions_; }

protected:
    SearchOptions options_;
    SearchStats stats_;
    ExclusionList exclusions_;
    mutable std::mutex results_mutex_;

    void logVerbose(const std::string& message) const {
        if (options_.verbose) {
            std::cout << std::format("[VERBOSE] {}\n", message);
        }
    }

    [[nodiscard]] bool shouldContinueSearch() const {
        if (options_.max_results) {
            return stats_.files_found < *options_.max_results;
        }
        return true;
    }
};

// Depth-Limited Search (DLS) implementation
class DLSSearcher : public FileSearcher {
public:
    using FileSearcher::FileSearcher;

    void search(const fs::path& root, const PatternMatcher& pattern, int max_depth) {
        stats_.start();

        if (options_.verbose) {
            std::cout << std::format("Starting DLS search from: {}\n", root.string());
            std::cout << std::format("Maximum depth: {}\n", max_depth);
        }

        dls(root, pattern, max_depth);

        stats_.stop();

        if (options_.show_statistics) {
            stats_.print();
        }
    }

private:
    void dls(const fs::path& current, const PatternMatcher& pattern, int depth) {
        if (!shouldContinueSearch()) return;

        if (exclusions_.shouldSkip(current)) {
            logVerbose(std::format("Skipping excluded path: {}", current.string()));
            return;
        }

        if (!options_.follow_symlinks && fs::is_symlink(current)) {
            logVerbose(std::format("Skipping symlink: {}", current.string()));
            return;
        }

        try {
            if (depth >= 0) {
                stats_.directories_scanned++;

                for (const auto& entry : fs::directory_iterator(current)) {
                    if (!shouldContinueSearch()) break;

                    if (entry.is_directory()) {
                        dls(entry.path(), pattern, depth - 1);
                    } else if (pattern.matches(entry.path().filename().string())) {
                        std::lock_guard lock(results_mutex_);
                        std::cout << std::format("Found: {}\n", entry.path().string());
                        stats_.files_found++;

                        if (options_.stop_on_first_match) break;
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            stats_.errors_encountered++;

            if (isPermissionDenied(e.code())) {
                logVerbose(std::format("Permission denied: {}", current.string()));
            } else {
                std::cerr << std::format("Error accessing {}: {}\n",
                    current.string(), e.what());
            }
        }
    }
};

// Iterative Deepening DFS (IDDFS) implementation
class IDDFSSearcher : public FileSearcher {
public:
    using FileSearcher::FileSearcher;

    void search(const fs::path& root, const PatternMatcher& pattern, int max_depth) {
        stats_.start();

        if (options_.verbose) {
            std::cout << std::format("Starting IDDFS search from: {}\n", root.string());
            std::cout << std::format("Maximum depth: {}\n", max_depth);
        }

        for (int depth = 0; depth <= max_depth; ++depth) {
            if (!shouldContinueSearch()) break;

            logVerbose(std::format("Searching at depth: {}", depth));

            DLSSearcher dls(options_);
            dls.exclusions() = exclusions_;
            dls.search(root, pattern, depth);

            // Aggregate statistics
            stats_.files_found += dls.stats().files_found;
            stats_.directories_scanned += dls.stats().directories_scanned;
            stats_.errors_encountered += dls.stats().errors_encountered;

            if (options_.stop_on_first_match && stats_.files_found > 0) {
                break;
            }
        }

        stats_.stop();

        if (options_.show_statistics) {
            stats_.print();
        }
    }
};

} // namespace dfs_search
