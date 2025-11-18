/**
 * @file iddfs_main.cpp
 * @brief Modern C++20/23 implementation of Iterative Deepening DFS File Search
 * @version 2.0.0
 * @date 2025
 *
 * This program implements IDDFS for file searching with modern C++ features.
 * IDDFS combines the space efficiency of DFS with the completeness of BFS.
 */

#include "../include/dfs_search.hpp"
#include <cxxopts.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    using namespace dfs_search;

    cxxopts::Options options("iddfs-search", "Iterative Deepening DFS file finder");

    options.add_options()
        ("p,pattern", "Search pattern", cxxopts::value<std::string>())
        ("d,directory", "Root directory to search", cxxopts::value<std::string>()->default_value("."))
        ("m,max-depth", "Maximum search depth", cxxopts::value<int>()->default_value("10"))
        ("r,regex", "Use regex pattern matching")
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
            std::cout << "IDDFS explores paths incrementally, finding files at shallower depths first.\n\n";
            std::cout << "Examples:\n";
            std::cout << "  " << argv[0] << " -p '*.conf' -d /etc -m 3\n";
            std::cout << "  " << argv[0] << " -p 'main\\..*' -r -v -f\n";
            return result.count("help") ? 0 : 1;
        }

        std::string pattern = result["pattern"].as<std::string>();
        std::string root_dir = result["directory"].as<std::string>();
        int max_depth = result["max-depth"].as<int>();

        fs::path root_path(root_dir);
        if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
            std::cerr << std::format("Error: '{}' is not a valid directory\n", root_dir);
            return 1;
        }

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

        PatternMatcher matcher(pattern, search_opts.match_type);
        IDDFSSearcher searcher(search_opts);

        if (result.count("exclude")) {
            auto exclusions = result["exclude"].as<std::vector<std::string>>();
            searcher.exclusions().addMultiple(exclusions);
        }

        std::cout << std::format("IDDFS search for '{}' in '{}' (max depth: {})\n\n",
            pattern, root_dir, max_depth);

        searcher.search(root_path, matcher, max_depth);

        if (searcher.stats().files_found == 0) {
            std::cout << "No matching files found.\n";
            return 1;
        }

        return 0;

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << std::format("Error parsing options: {}\n", e.what());
        return 1;
    } catch (const std::exception& e) {
        std::cerr << std::format("Error: {}\n", e.what());
        return 1;
    }
}
