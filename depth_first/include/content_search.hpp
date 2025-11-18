#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <regex>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace dfs_search {

namespace fs = std::filesystem;

/**
 * @brief Content match result
 */
struct ContentMatch {
    fs::path file_path;
    size_t line_number{0};
    size_t column{0};
    std::string line_content;
    std::string matched_text;
};

/**
 * @brief Memory-mapped file for fast content searching
 */
class MemoryMappedFile {
public:
    /**
     * @brief Open and memory-map a file
     */
    explicit MemoryMappedFile(const fs::path& file_path)
        : fd_(-1), data_(nullptr), size_(0) {

        fd_ = open(file_path.c_str(), O_RDONLY);
        if (fd_ == -1) {
            return;
        }

        struct stat sb;
        if (fstat(fd_, &sb) == -1) {
            close(fd_);
            fd_ = -1;
            return;
        }

        size_ = sb.st_size;

        if (size_ == 0) {
            // Empty file
            close(fd_);
            fd_ = -1;
            return;
        }

        data_ = static_cast<char*>(mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0));

        if (data_ == MAP_FAILED) {
            data_ = nullptr;
            close(fd_);
            fd_ = -1;
        }
    }

    ~MemoryMappedFile() {
        if (data_ != nullptr) {
            munmap(data_, size_);
        }
        if (fd_ != -1) {
            close(fd_);
        }
    }

    // Delete copy, allow move
    MemoryMappedFile(const MemoryMappedFile&) = delete;
    MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;

    MemoryMappedFile(MemoryMappedFile&& other) noexcept
        : fd_(other.fd_), data_(other.data_), size_(other.size_) {
        other.fd_ = -1;
        other.data_ = nullptr;
        other.size_ = 0;
    }

    MemoryMappedFile& operator=(MemoryMappedFile&& other) noexcept {
        if (this != &other) {
            if (data_ != nullptr) {
                munmap(data_, size_);
            }
            if (fd_ != -1) {
                close(fd_);
            }

            fd_ = other.fd_;
            data_ = other.data_;
            size_ = other.size_;

            other.fd_ = -1;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] bool is_open() const { return data_ != nullptr; }
    [[nodiscard]] const char* data() const { return data_; }
    [[nodiscard]] size_t size() const { return size_; }

private:
    int fd_;
    char* data_;
    size_t size_;
};

/**
 * @brief Content searcher using memory-mapped files
 */
class ContentSearcher {
public:
    /**
     * @brief Search for pattern in file (simple string search)
     */
    static std::vector<ContentMatch> search(const fs::path& file_path, const std::string& pattern) {
        std::vector<ContentMatch> matches;

        MemoryMappedFile mmap(file_path);
        if (!mmap.is_open()) {
            return matches;
        }

        const char* data = mmap.data();
        size_t size = mmap.size();

        size_t line_num = 1;
        size_t line_start = 0;

        for (size_t i = 0; i < size; ++i) {
            // Check for newline
            if (data[i] == '\n') {
                // Search in current line
                std::string line(data + line_start, i - line_start);
                size_t pos = line.find(pattern);

                if (pos != std::string::npos) {
                    ContentMatch match;
                    match.file_path = file_path;
                    match.line_number = line_num;
                    match.column = pos;
                    match.line_content = line;
                    match.matched_text = pattern;
                    matches.push_back(match);
                }

                line_num++;
                line_start = i + 1;
            }
        }

        // Handle last line if no trailing newline
        if (line_start < size) {
            std::string line(data + line_start, size - line_start);
            size_t pos = line.find(pattern);

            if (pos != std::string::npos) {
                ContentMatch match;
                match.file_path = file_path;
                match.line_number = line_num;
                match.column = pos;
                match.line_content = line;
                match.matched_text = pattern;
                matches.push_back(match);
            }
        }

        return matches;
    }

    /**
     * @brief Search for regex pattern in file
     */
    static std::vector<ContentMatch> search_regex(const fs::path& file_path, const std::regex& pattern) {
        std::vector<ContentMatch> matches;

        MemoryMappedFile mmap(file_path);
        if (!mmap.is_open()) {
            return matches;
        }

        const char* data = mmap.data();
        size_t size = mmap.size();

        size_t line_num = 1;
        size_t line_start = 0;

        for (size_t i = 0; i < size; ++i) {
            if (data[i] == '\n') {
                std::string line(data + line_start, i - line_start);
                std::smatch sm;

                if (std::regex_search(line, sm, pattern)) {
                    ContentMatch match;
                    match.file_path = file_path;
                    match.line_number = line_num;
                    match.column = sm.position();
                    match.line_content = line;
                    match.matched_text = sm.str();
                    matches.push_back(match);
                }

                line_num++;
                line_start = i + 1;
            }
        }

        // Handle last line
        if (line_start < size) {
            std::string line(data + line_start, size - line_start);
            std::smatch sm;

            if (std::regex_search(line, sm, pattern)) {
                ContentMatch match;
                match.file_path = file_path;
                match.line_number = line_num;
                match.column = sm.position();
                match.line_content = line;
                match.matched_text = sm.str();
                matches.push_back(match);
            }
        }

        return matches;
    }

    /**
     * @brief Check if file contains pattern (faster, stops at first match)
     */
    static bool contains(const fs::path& file_path, const std::string& pattern) {
        MemoryMappedFile mmap(file_path);
        if (!mmap.is_open()) {
            return false;
        }

        const char* data = mmap.data();
        size_t size = mmap.size();

        // Use memmem for fast substring search
        const void* result = memmem(data, size, pattern.c_str(), pattern.size());
        return result != nullptr;
    }
};

} // namespace dfs_search
