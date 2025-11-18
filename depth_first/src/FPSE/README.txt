This C++ program is designed to perform a recursive search for files in a specified directory and its subdirectories, based on a given pattern. It also allows you to limit the depth of the search within subdirectories. The program takes three command-line arguments: the target pattern, the root directory to start the search from, and the maximum depth for the search.

Here's a step-by-step explanation of how the program works:

Header Inclusions: The program starts by including several C++ standard library headers, including <iostream> for input/output, <filesystem> for file system operations, <string> for string handling, and <vector> for storing excluded paths.

Namespace Alias: A namespace alias fs is defined for the std::filesystem namespace to make it easier to work with file system functions.

matchesPattern Function: This function takes a filename and a pattern as input and determines whether the filename matches the pattern. It is used to filter files based on their names. The function handles two cases:

If the pattern is "*", it matches any filename, and the function returns true.
If the pattern starts with "*." (e.g., "*.txt"), it matches filenames with the specified file extension. If the filename's extension matches the pattern, the function returns true.
shouldSkipDirectory Function: This function checks whether a given directory path should be skipped during the search. It contains a list of excluded paths that should not be traversed during the search. If the input path matches any of the excluded paths, the function returns true, indicating that the directory should be skipped.

DLS Function: The DLS function (Depth-Limited Search) performs the recursive search for files within a directory and its subdirectories. It takes the following parameters:

currentPath: The current directory being processed.
targetPattern: The pattern to match filenames against.
depth: The current depth in the directory tree.
found: A boolean reference indicating whether a matching file has been found.
Inside the function:

It first checks whether the current directory should be skipped using shouldSkipDirectory and whether it's a symbolic link using fs::is_symlink. If either condition is met, the function returns without further processing.
If the depth is greater than zero, it iterates through the entries in the current directory using fs::directory_iterator.
For each entry:
If it's a directory, a recursive call to DLS is made with the subdirectory as the new currentPath.
If it's a file and matches the target pattern using matchesPattern, it prints a message indicating that the file was found and sets the found flag to true.
recursiveSearch Function: This function serves as the entry point for the search operation. It takes the following parameters:

rootPath: The root directory from which the search starts.
targetPattern: The pattern to match filenames against.
maxDepth: The maximum depth to search within subdirectories.
Inside the function:

It initializes a found flag to false.
It calls the DLS function with the initial parameters, starting the recursive search.
If no matching files are found (found is still false), it prints a message indicating that no files matching the pattern were found.
main Function: The main function is the program's entry point. It parses command-line arguments and orchestrates the search operation:

It checks if the program was provided with exactly 4 command-line arguments (including the program name).
It extracts the target pattern, root directory, and maximum depth from the command-line arguments.
It checks if the specified root directory exists and is indeed a directory.
If all checks pass, it calls the recursiveSearch function to start the search operation.
The program returns 0 to indicate successful execution if the search completes.
Overall, this program provides a flexible way to search for files that match a specific pattern within a directory structure while allowing control over the search depth. It also skips specified excluded paths during the search.









It seems there's a discrepancy between the compiled program (FPSEv1) and the executed program (FilePatternSearchExcluded). The FPSEv1 program expects three arguments as per its usage instructions, while FilePatternSearchExcluded expects two arguments based on its error message. This suggests you might have compiled one version of the program (with the max depth parameter) and are attempting to run a different version (without the max depth parameter).

To resolve this, ensure you are compiling and running the same version of the program. Based on your needs, you should decide whether you want to include the max depth parameter or not. If you include it, the program will limit the depth of its search to the specified number. If you exclude it, the program will search to the maximum depth possible.

For clarity, here's the version of the program with the max depth parameter, compiled and executed with the same name (FPSEv1):

FPSEv1.cpp with Max Depth Parameter
cpp
Copy code
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
Compilation and Execution
Compile the program:

bash
Copy code
g++ -std=c++17 -o FPSEv1 FPSEv1.cpp
Execute the compiled program:

bash
Copy code
./FPSEv1 "*.txt" /home/walter 10
Ensure that the name used in the compilation command (FPSEv1 in this case) matches the name used when executing the program. This will help avoid confusion and ensure that the correct version of the program is being run.
