#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

bool matchesPattern(const std::string& filename, const std::string& pattern) {
    if (pattern == "*") return true;
    if (pattern.size() > 1 && pattern[0] == '*' && pattern[1] == '.') {
        std::string ext = pattern.substr(1);
        if (filename.size() >= ext.size() && filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0) {
            return true;
        }
    }
    return false;
}

bool shouldSkipDirectory(const fs::path& path) {
    static const std::vector<std::string> excludedPaths = {
        "/proc",
        "/sys",
        "/tmp",
        "/home/walter/.steam",
        "/home/walter/.local"
        // Add other paths to exclude here
    };

    std::string pathStr = path.string();
    for (const auto& excludedPath : excludedPaths) {
        if (pathStr.find(excludedPath) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void DLS(const fs::path& currentPath, const std::string& targetPattern, int depth, bool& found) {
    if (shouldSkipDirectory(currentPath)) {
        return;
    }
    try {
        for (const auto& dirEntry : fs::directory_iterator(currentPath)) {
            if (dirEntry.is_directory() && depth > 0) {
                DLS(dirEntry.path(), targetPattern, depth - 1, found);
            } else if (matchesPattern(dirEntry.path().filename().string(), targetPattern)) {
                std::cout << "Found file: " << dirEntry.path() << std::endl;
                found = true;
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cout << "Error reading directory '" << currentPath << "': " << e.what() << std::endl;
    }
}

void recursiveSearch(const fs::path& rootPath, const std::string& targetPattern) {
    const int MAX_DEPTH = std::numeric_limits<int>::max();
    bool found = false;
    DLS(rootPath, targetPattern, MAX_DEPTH, found);
    if (!found) {
        std::cout << "No files matching the pattern were found." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <pattern> <rootDir>\n";
        std::cout << "Example: " << argv[0] << " *.txt /home/user\n";
        return 1;
    }

    std::string targetPattern = argv[1];
    fs::path rootPath = argv[2];

    if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
        std::cout << "Root directory does not exist or is not a directory: " << rootPath << std::endl;
        return 1;
    }

    recursiveSearch(rootPath, targetPattern);

    return 0;
}
