#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

// Function to check if a filename matches a wildcard pattern (only works for patterns like "*.ext")
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

// Recursive function to perform Depth-Limited Search
bool DLS(const fs::path& currentPath, const std::string& targetPattern, int depth) {
    try {
        if (depth >= 0) {
            for (const auto& dirEntry : fs::directory_iterator(currentPath)) {
                if (depth == 0 && matchesPattern(dirEntry.path().filename().string(), targetPattern)) {
                    std::cout << "Found file: " << dirEntry.path() << std::endl;
                    return true;
                }
                else if (dirEntry.is_directory()) {
                    if (DLS(dirEntry.path(), targetPattern, depth - 1)) {
                        return true;
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cout << "Error reading directory '" << currentPath << "': " << e.what() << std::endl;
    }
    return false;
}

// Function to perform Iterative Deepening Depth-First Search up to a maximum depth
void IDDFS(const fs::path& rootPath, const std::string& targetPattern, int maxDepth) {
    for (int depth = 0; depth <= maxDepth; depth++) {
        if (DLS(rootPath, targetPattern, depth)) {
            return;
        }
    }
    std::cout << "File not found within depth " << maxDepth << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <pattern> <rootDir> <maxDepth>\n";
        std::cout << "Example: " << argv[0] << " *.txt /home/user 5\n";
        return 1;
    }

    std::string targetPattern = argv[1];
    fs::path rootPath = argv[2];
    int maxDepth = std::stoi(argv[3]);

    if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
        std::cout << "Root directory does not exist or is not a directory: " << rootPath << std::endl;
        return 1;
    }

    IDDFS(rootPath, targetPattern, maxDepth);

    return 0;
}
