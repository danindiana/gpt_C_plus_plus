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
        "/home/walter/.local",
        // Add other paths to exclude here
        // Example of a deeply nested path
        "/media/walter/462C364D2C3637EF/Users/walter/AppData/Local/Application Data"
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
    if (shouldSkipDirectory(currentPath) || fs::is_symlink(currentPath)) {
        return;
    }
    try {
        if (depth > 0) { // Adjusted depth condition
            for (const auto& dirEntry : fs::directory_iterator(currentPath)) {
                if (dirEntry.is_directory()) {
                    DLS(dirEntry.path(), targetPattern, depth - 1, found);
                } else if (matchesPattern(dirEntry.path().filename().string(), targetPattern)) {
                    std::cout << "Found file: " << dirEntry.path() << std::endl;
                    found = true;
                }
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cout << "Error reading directory '" << currentPath << "': " << e.what() << std::endl;
    }
}

void recursiveSearch(const fs::path& rootPath, const std::string& targetPattern, int maxDepth) {
    bool found = false;
    DLS(rootPath, targetPattern, maxDepth, found);
    if (!found) {
        std::cout << "No files matching the pattern were found." << std::endl;
    }
}

// ... [previous code remains the same]

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <pattern> <rootDir> <maxDepth>\n";
        std::cout << "Example: " << argv[0] << " *.txt /home/user 10\n";
        return 1;
    }

    std::string targetPattern = argv[1];
    fs::path rootPath = argv[2];
    int maxDepth = std::stoi(argv[3]);

    if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
        std::cout << "Root directory does not exist or is not a directory: " << rootPath << std::endl;
        return 1;
    }

    recursiveSearch(rootPath, targetPattern, maxDepth);

    return 0;
}
