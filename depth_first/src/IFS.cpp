#include <iostream>
#include <filesystem>
#include <string>

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

bool DLS(const fs::path& currentPath, const std::string& targetPattern, int depth) {
    try {
        if (depth >= 0) {
            for (const auto& dirEntry : fs::directory_iterator(currentPath)) {
                if (matchesPattern(dirEntry.path().filename().string(), targetPattern)) {
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

void recursiveSearch(const fs::path& rootPath, const std::string& targetPattern) {
    const int MAX_DEPTH = std::numeric_limits<int>::max();
    DLS(rootPath, targetPattern, MAX_DEPTH);
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
