#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

namespace dfs_search {

namespace fs = std::filesystem;

/**
 * @brief File hash calculator for duplicate detection
 *
 * Uses SHA-256 for robust hash calculation
 */
class FileHasher {
public:
    /**
     * @brief Calculate SHA-256 hash of a file
     * @param file_path Path to file
     * @return Hex-encoded hash string, or empty on error
     */
    static std::string calculate_hash(const fs::path& file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        SHA256_CTX sha256_ctx;
        SHA256_Init(&sha256_ctx);

        constexpr size_t buffer_size = 8192;
        std::vector<char> buffer(buffer_size);

        while (file.read(buffer.data(), buffer_size) || file.gcount() > 0) {
            SHA256_Update(&sha256_ctx, buffer.data(), file.gcount());
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256_ctx);

        return bytes_to_hex(hash, SHA256_DIGEST_LENGTH);
    }

    /**
     * @brief Quick hash using file size and first/last bytes
     * Much faster than full hash, good for initial filtering
     */
    static std::string quick_hash(const fs::path& file_path) {
        std::error_code ec;
        auto size = fs::file_size(file_path, ec);
        if (ec) {
            return "";
        }

        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        constexpr size_t sample_size = 1024;
        std::vector<char> head(sample_size);
        std::vector<char> tail(sample_size);

        // Read first bytes
        file.read(head.data(), sample_size);
        size_t head_read = file.gcount();

        // Read last bytes if file is large enough
        size_t tail_read = 0;
        if (size > sample_size * 2) {
            file.seekg(-static_cast<std::streamoff>(sample_size), std::ios::end);
            file.read(tail.data(), sample_size);
            tail_read = file.gcount();
        }

        // Create hash from size + samples
        SHA256_CTX sha256_ctx;
        SHA256_Init(&sha256_ctx);

        SHA256_Update(&sha256_ctx, &size, sizeof(size));
        SHA256_Update(&sha256_ctx, head.data(), head_read);
        SHA256_Update(&sha256_ctx, tail.data(), tail_read);

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &sha256_ctx);

        return bytes_to_hex(hash, SHA256_DIGEST_LENGTH);
    }

private:
    static std::string bytes_to_hex(const unsigned char* bytes, size_t length) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (size_t i = 0; i < length; ++i) {
            oss << std::setw(2) << static_cast<int>(bytes[i]);
        }

        return oss.str();
    }
};

/**
 * @brief Duplicate file detector
 */
class DuplicateDetector {
public:
    /**
     * @brief Add file to duplicate detection
     * @param file_path Path to file
     * @return true if file is a duplicate
     */
    bool add_file(const fs::path& file_path) {
        // First check by size
        std::error_code ec;
        auto size = fs::file_size(file_path, ec);
        if (ec) {
            return false;
        }

        // Quick filter: if size is unique, not a duplicate
        if (size_map_[size].empty()) {
            size_map_[size].push_back(file_path);
            return false;
        }

        // Size collision - calculate quick hash
        std::string qhash = FileHasher::quick_hash(file_path);
        if (qhash.empty()) {
            return false;
        }

        // Check if quick hash exists
        if (quick_hash_map_.count(qhash) > 0) {
            // Potential duplicate - verify with full hash
            std::string full_hash = FileHasher::calculate_hash(file_path);
            if (full_hash.empty()) {
                return false;
            }

            if (full_hash_map_.count(full_hash) > 0) {
                // Confirmed duplicate
                duplicates_[full_hash].push_back(file_path);
                return true;
            } else {
                // Not a duplicate, add to maps
                full_hash_map_[full_hash] = file_path;
                quick_hash_map_[qhash] = file_path;
                size_map_[size].push_back(file_path);
                return false;
            }
        } else {
            // New quick hash
            quick_hash_map_[qhash] = file_path;
            size_map_[size].push_back(file_path);
            return false;
        }
    }

    /**
     * @brief Get all duplicate groups
     */
    const std::unordered_map<std::string, std::vector<fs::path>>& get_duplicates() const {
        return duplicates_;
    }

    /**
     * @brief Get number of duplicate groups
     */
    size_t duplicate_count() const {
        return duplicates_.size();
    }

    /**
     * @brief Clear all data
     */
    void clear() {
        size_map_.clear();
        quick_hash_map_.clear();
        full_hash_map_.clear();
        duplicates_.clear();
    }

private:
    // Map of file size -> list of files with that size
    std::unordered_map<uintmax_t, std::vector<fs::path>> size_map_;

    // Map of quick hash -> first file with that hash
    std::unordered_map<std::string, fs::path> quick_hash_map_;

    // Map of full hash -> first file with that hash
    std::unordered_map<std::string, fs::path> full_hash_map_;

    // Map of full hash -> list of duplicate files
    std::unordered_map<std::string, std::vector<fs::path>> duplicates_;
};

} // namespace dfs_search
