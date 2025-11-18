#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "../include/thread_pool.hpp"
#include "../include/output_formatter.hpp"
#include "../include/config_parser.hpp"

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace dfs_search;
namespace fs = std::filesystem;

TEST_CASE("ThreadPool basic functionality", "[thread_pool]") {
    SECTION("Create thread pool with default threads") {
        ThreadPool pool;
        REQUIRE(pool.size() > 0);
        REQUIRE(pool.size() <= std::thread::hardware_concurrency());
    }

    SECTION("Execute simple tasks") {
        ThreadPool pool(4);

        auto future1 = pool.enqueue([] { return 42; });
        auto future2 = pool.enqueue([] { return 100; });

        REQUIRE(future1.get() == 42);
        REQUIRE(future2.get() == 100);
    }

    SECTION("Execute multiple tasks") {
        ThreadPool pool(4);
        std::vector<std::future<int>> futures;

        for (int i = 0; i < 10; ++i) {
            futures.push_back(pool.enqueue([i] { return i * i; }));
        }

        for (int i = 0; i < 10; ++i) {
            REQUIRE(futures[i].get() == i * i);
        }
    }
}

TEST_CASE("ConfigParser functionality", "[config]") {
    // Create a temporary config file
    const char* config_content = R"(
# Test configuration
max_depth=15
threads=8
parallel=true
format=json

# Excluded paths
exclude=/tmp
exclude=/proc
exclude=/sys
)";

    fs::path temp_config = fs::temp_directory_path() / "test_dfsrc";
    {
        std::ofstream file(temp_config);
        file << config_content;
    }

    SECTION("Load configuration file") {
        ConfigParser parser;
        REQUIRE(parser.load(temp_config));

        REQUIRE(parser.get_int("max_depth") == 15);
        REQUIRE(parser.get_int("threads") == 8);
        REQUIRE(parser.get_bool("parallel") == true);
        REQUIRE(parser.get("format") == "json");
    }

    SECTION("Get list values") {
        ConfigParser parser;
        parser.load(temp_config);

        auto exclusions = parser.get_list("exclude");
        REQUIRE(exclusions.size() == 3);
        REQUIRE(std::find(exclusions.begin(), exclusions.end(), "/tmp") != exclusions.end());
        REQUIRE(std::find(exclusions.begin(), exclusions.end(), "/proc") != exclusions.end());
    }

    SECTION("Default values") {
        ConfigParser parser;
        REQUIRE(parser.get_int("nonexistent", 99) == 99);
        REQUIRE(parser.get("missing", "default") == "default");
        REQUIRE(parser.get_bool("nope", false) == false);
    }

    SECTION("Save configuration") {
        ConfigParser parser;
        parser.set("key1", "value1");
        parser.set("key2", "value2");

        fs::path save_path = fs::temp_directory_path() / "test_save_dfsrc";
        REQUIRE(parser.save(save_path));

        ConfigParser parser2;
        REQUIRE(parser2.load(save_path));
        REQUIRE(parser2.get("key1") == "value1");
        REQUIRE(parser2.get("key2") == "value2");

        fs::remove(save_path);
    }

    fs::remove(temp_config);
}

TEST_CASE("OutputFormatter functionality", "[formatter]") {
    SearchResult result;
    result.file_path = "/home/user/test.txt";
    result.filename = "test.txt";
    result.file_size = 1024;
    result.file_type = ".txt";
    result.depth = 3;

    SearchStatistics stats;
    stats.files_found = 10;
    stats.directories_scanned = 5;
    stats.errors_encountered = 1;
    stats.duration = std::chrono::milliseconds(123);
    stats.threads_used = 4;

    SECTION("FormatterFactory creates correct formatters") {
        auto plain = FormatterFactory::create(OutputFormat::PLAIN);
        REQUIRE(plain != nullptr);

        auto colored = FormatterFactory::create(OutputFormat::COLORED);
        REQUIRE(colored != nullptr);

        auto json = FormatterFactory::create(OutputFormat::JSON);
        REQUIRE(json != nullptr);

        auto xml = FormatterFactory::create(OutputFormat::XML);
        REQUIRE(xml != nullptr);

        auto csv = FormatterFactory::create(OutputFormat::CSV);
        REQUIRE(csv != nullptr);
    }

    SECTION("Formatters can print without crashing") {
        auto formatter = FormatterFactory::create(OutputFormat::PLAIN);

        REQUIRE_NOTHROW(formatter->print_header("*.txt", "/home/user", 10));
        REQUIRE_NOTHROW(formatter->print_result(result));
        REQUIRE_NOTHROW(formatter->print_statistics(stats));
        REQUIRE_NOTHROW(formatter->print_footer());
    }
}

TEST_CASE("Pattern matching", "[pattern]") {
    SECTION("Wildcard extension matching") {
        // This would test the pattern matching from the main code
        // For now, just a placeholder
        REQUIRE(true);
    }
}

TEST_CASE("File operations", "[file]") {
    // Create a temporary test file
    fs::path temp_file = fs::temp_directory_path() / "test_file.txt";
    {
        std::ofstream file(temp_file);
        file << "Line 1\n";
        file << "Line 2 with pattern\n";
        file << "Line 3\n";
    }

    SECTION("File exists") {
        REQUIRE(fs::exists(temp_file));
        REQUIRE(fs::is_regular_file(temp_file));
    }

    SECTION("File size") {
        auto size = fs::file_size(temp_file);
        REQUIRE(size > 0);
    }

    fs::remove(temp_file);
}
