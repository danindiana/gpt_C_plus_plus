#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace dfs_search {

namespace fs = std::filesystem;

/**
 * @brief Configuration parser for .dfsrc files
 *
 * Supports simple key=value format with comments
 * Example:
 *   # Default search depth
 *   max_depth=10
 *
 *   # Paths to exclude
 *   exclude=/proc
 *   exclude=/sys
 */
class ConfigParser {
public:
    /**
     * @brief Load configuration from file
     * @param config_file Path to configuration file
     * @return true if loaded successfully
     */
    bool load(const fs::path& config_file) {
        if (!fs::exists(config_file)) {
            return false;
        }

        std::ifstream file(config_file);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        int line_num = 0;

        while (std::getline(file, line)) {
            line_num++;

            // Trim whitespace
            line = trim(line);

            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                continue;
            }

            // Parse key=value
            auto eq_pos = line.find('=');
            if (eq_pos == std::string::npos) {
                std::cerr << "Warning: Invalid config line " << line_num << ": " << line << "\n";
                continue;
            }

            std::string key = trim(line.substr(0, eq_pos));
            std::string value = trim(line.substr(eq_pos + 1));

            // Handle multi-value keys (like exclude)
            if (config_.count(key) > 0) {
                // Append to existing value with comma separator
                config_[key] += "," + value;
            } else {
                config_[key] = value;
            }
        }

        return true;
    }

    /**
     * @brief Load from default locations
     * Searches: ./.dfsrc, ~/.dfsrc, /etc/dfsrc
     */
    bool load_defaults() {
        std::vector<fs::path> search_paths = {
            "./.dfsrc",
            fs::path(std::getenv("HOME") ? std::getenv("HOME") : ".") / ".dfsrc",
            "/etc/dfsrc"
        };

        for (const auto& path : search_paths) {
            if (load(path)) {
                return true;
            }
        }

        return false;
    }

    /**
     * @brief Get string value
     */
    std::string get(const std::string& key, const std::string& default_value = "") const {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : default_value;
    }

    /**
     * @brief Get integer value
     */
    int get_int(const std::string& key, int default_value = 0) const {
        auto str_value = get(key);
        if (str_value.empty()) {
            return default_value;
        }

        try {
            return std::stoi(str_value);
        } catch (...) {
            return default_value;
        }
    }

    /**
     * @brief Get boolean value
     */
    bool get_bool(const std::string& key, bool default_value = false) const {
        auto str_value = get(key);
        if (str_value.empty()) {
            return default_value;
        }

        // Convert to lowercase
        std::string lower = str_value;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        return (lower == "true" || lower == "yes" || lower == "1" || lower == "on");
    }

    /**
     * @brief Get list of values (comma-separated)
     */
    std::vector<std::string> get_list(const std::string& key) const {
        std::vector<std::string> result;
        auto str_value = get(key);

        if (str_value.empty()) {
            return result;
        }

        std::istringstream iss(str_value);
        std::string item;

        while (std::getline(iss, item, ',')) {
            item = trim(item);
            if (!item.empty()) {
                result.push_back(item);
            }
        }

        return result;
    }

    /**
     * @brief Check if key exists
     */
    bool has(const std::string& key) const {
        return config_.count(key) > 0;
    }

    /**
     * @brief Set a value
     */
    void set(const std::string& key, const std::string& value) {
        config_[key] = value;
    }

    /**
     * @brief Save configuration to file
     */
    bool save(const fs::path& config_file) const {
        std::ofstream file(config_file);
        if (!file.is_open()) {
            return false;
        }

        file << "# DFS Search Configuration\n";
        file << "# Generated configuration file\n\n";

        for (const auto& [key, value] : config_) {
            // Handle multi-value keys
            if (value.find(',') != std::string::npos) {
                std::istringstream iss(value);
                std::string item;
                while (std::getline(iss, item, ',')) {
                    item = trim(item);
                    if (!item.empty()) {
                        file << key << "=" << item << "\n";
                    }
                }
            } else {
                file << key << "=" << value << "\n";
            }
        }

        return true;
    }

    /**
     * @brief Get all configuration
     */
    const std::unordered_map<std::string, std::string>& get_all() const {
        return config_;
    }

private:
    std::unordered_map<std::string, std::string> config_;

    static std::string trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return "";
        }

        auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
};

} // namespace dfs_search
