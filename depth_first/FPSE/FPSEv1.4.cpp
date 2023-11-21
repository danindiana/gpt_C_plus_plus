#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <system_error>  // Include for std::error_code

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

bool isPermissionDenied(const std::error_code& ec) {
    // Check if the error code corresponds to a permission denied error
    return ec.value() == EACCES || ec.value() == EPERM;
}

void DLS(const fs::path& currentPath, const std::string& targetPattern, int depth, bool& found) {
    try {
        if (depth >= 0) {
            for (const auto& dirEntry : fs::directory_iterator(currentPath)) {
                if (dirEntry.is_directory()) {
                    DLS(dirEntry.path(), targetPattern, depth - 1, found);
                } else if (matchesPattern(dirEntry.path().filename().string(), targetPattern)) {
                    std::cout << "Found file: " << dirEntry.path() << std::endl;
                    found = true;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::error_code ec = e.code();
        if (isPermissionDenied(ec)) {
            std::cout << "Skipping directory due to permission denied: " << currentPath << std::endl;
        } else {
            std::cout << "Error reading directory '" << currentPath << "': " << e.what() << std::endl;
        }
    }
}

void recursiveSearch(const fs::path& rootPath, const std::string& targetPattern, int maxDepth) {
    bool found = false;
    DLS(rootPath, targetPattern, maxDepth, found);
    if (!found) {
        std::cout << "No files matching the pattern were found." << std::endl;
    }
}

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
