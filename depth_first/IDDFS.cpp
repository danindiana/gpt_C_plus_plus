#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

// Function to check if a filename matches a wildcard pattern (only works for patterns like "*.ext")
bool matchesPattern(const std::string& filename, const std::string& pattern) {
    if (pattern == "*") return true; // Match all files
    if (pattern.size() > 1 && pattern[0] == '*' && pattern[1] == '.') {
        std::string ext = pattern.substr(1); // Get the extension
        if (filename.size() >= ext.size() && filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0) {
            return true; // If the file ends with the given extension
        }
    }
    return false;
}

// Recursive function to perform Depth-Limited Search
bool DLS(const std::filesystem::path& currentPath, const std::string& targetPattern, int depth) {
    try {
        if (depth >= 0) {
            for (const auto& dirEntry : std::filesystem::directory_iterator(currentPath)) {
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
    catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Error reading directory '" << currentPath << "': " << e.what() << std::endl;
    }
    return false;
}

// Function to perform Iterative Deepening Depth-First Search up to a maximum depth
void IDDFS(const std::filesystem::path& rootPath, const std::string& targetPattern, int maxDepth) {
    for (int depth = 0; depth <= maxDepth; depth++) {
        if (DLS(rootPath, targetPattern, depth)) {
            return;
        }
    }
    std::cout << "File not found within depth " << maxDepth << std::endl;
}

int main() {
    std::string targetPattern;
    std::string rootDir;
    int maxDepth;

    // Get user input
    std::cout << "Enter file pattern to search (e.g., *.txt): ";
    std::getline(std::cin, targetPattern);

    std::cout << "Enter root directory to start search: ";
    std::getline(std::cin, rootDir);

    std::cout << "Enter maximum depth for search: ";
    std::cin >> maxDepth;

    std::filesystem::path rootPath = rootDir; // Your root path

    IDDFS(rootPath, targetPattern, maxDepth);

    return 0;
}
