/**
 * @file dfs_main.cpp
 * @brief Modern C++20/23 implementation of Depth-First File Search
 * @version 2.0.0
 * @date 2025
 *
 * This program implements a depth-limited file search using modern C++ features:
 * - C++20 concepts and ranges
 * - std::format for output
 * - Improved pattern matching with regex support
 * - Better error handling
 * - Search statistics
 */

#include "../include/dfs_search.hpp"
#include <cxxopts.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    using namespace dfs_search;

    // Command-line argument parsing with cxxopts
    cxxopts::Options options("dfs-search", "Depth-First Search file finder with modern C++20/23");

    options.add_options()
        ("p,pattern", "Search pattern (e.g., *.txt, *.cpp)", cxxopts::value<std::string>())
        ("d,directory", "Root directory to search", cxxopts::value<std::string>()->default_value("."))
        ("m,max-depth", "Maximum search depth", cxxopts::value<int>()->default_value("10"))
        ("r,regex", "Use regex pattern matching instead of wildcards")
        ("v,verbose", "Verbose output")
        ("s,follow-symlinks", "Follow symbolic links")
        ("f,first", "Stop on first match")
        ("l,limit", "Maximum number of results", cxxopts::value<size_t>())
        ("e,exclude", "Additional paths to exclude", cxxopts::value<std::vector<std::string>>())
        ("no-stats", "Don't show search statistics")
        ("h,help", "Print usage");

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help") || !result.count("pattern")) {
            std::cout << options.help() << "\n\n";
            std::cout << "Examples:\n";
            std::cout << "  " << argv[0] << " -p '*.txt' -d /home/user\n";
            std::cout << "  " << argv[0] << " -p 'test.*\\.cpp' -r -v\n";
            std::cout << "  " << argv[0] << " -p '*.log' -m 5 -f\n";
            return result.count("help") ? 0 : 1;
        }

        // Parse arguments
        std::string pattern = result["pattern"].as<std::string>();
        std::string root_dir = result["directory"].as<std::string>();
        int max_depth = result["max-depth"].as<int>();

        // Validate root directory
        fs::path root_path(root_dir);
        if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
            std::cerr << std::format("Error: '{}' is not a valid directory\n", root_dir);
            return 1;
        }

        // Configure search options
        SearchOptions search_opts;
        search_opts.verbose = result.count("verbose") > 0;
        search_opts.follow_symlinks = result.count("follow-symlinks") > 0;
        search_opts.stop_on_first_match = result.count("first") > 0;
        search_opts.show_statistics = result.count("no-stats") == 0;

        if (result.count("limit")) {
            search_opts.max_results = result["limit"].as<size_t>();
        }

        if (result.count("regex")) {
            search_opts.match_type = PatternMatcher::MatchType::REGEX;
        }

        // Create pattern matcher
        PatternMatcher matcher(pattern, search_opts.match_type);

        // Create and configure searcher
        DLSSearcher searcher(search_opts);

        if (result.count("exclude")) {
            auto exclusions = result["exclude"].as<std::vector<std::string>>();
            searcher.exclusions().addMultiple(exclusions);
        }

        // Perform search
        std::cout << std::format("Searching for '{}' in '{}' (max depth: {})\n\n",
            pattern, root_dir, max_depth);

        searcher.search(root_path, matcher, max_depth);

        if (searcher.stats().files_found == 0) {
            std::cout << "No matching files found.\n";
            return 1;
        }

        return 0;

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << std::format("Error parsing options: {}\n", e.what());
        std::cerr << "Use --help for usage information\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
        return 1;
    }
}
