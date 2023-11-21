Recommended Program to use in search: FilePatternSearchExcluded.cpp Nov-21-2023

This C++ program performs a file search based on a specified pattern, starting from a given root directory and searching up to a specified depth. It implements an Iterative Deepening Depth-First Search (IDDFS) algorithm. Here's a breakdown of how it functions:

Program Functionality
User Input:

The program prompts the user to input three pieces of information: a file pattern to search for (e.g., *.txt), a root directory to start the search from, and a maximum depth for the search.
Pattern Matching:

The matchesPattern function checks if a filename matches a given wildcard pattern. It is designed to handle simple patterns like "*.ext", where "*" matches any string and ".ext" is a specific file extension.
Depth-Limited Search (DLS):

DLS is a recursive function that performs a Depth-Limited Search in the filesystem. It iterates through each entry in a directory:
If it finds a file matching the pattern at the current depth, it prints the file's path and stops the search.
If it encounters a directory and hasn't reached the depth limit, it recursively searches in that directory.
Iterative Deepening Depth-First Search (IDDFS):

IDDFS performs the Depth-First Search iteratively, starting at depth 0 and incrementing the depth up to a maximum depth specified by the user.
If the file is found at any depth, the search stops; otherwise, it continues until the maximum depth is reached.
Error Handling:

The program handles filesystem errors (such as inaccessible directories) and reports them to the user.
Suggestions for Making the Program Easier to Use
To make the program more user-friendly for someone working from the Ubuntu CLI environment, consider the following enhancements:

Default Values:

Provide default values for root directory (e.g., current directory) and maximum depth, so the user doesn't always have to input them.
Command-Line Arguments:

Instead of asking for inputs interactively, modify the program to take command-line arguments. This makes it easier to use the program in scripts or batch processes.
Help Message:

Include a help message that explains how to use the program and what each argument means. This can be displayed when the program is run with a -h or --help flag.
Verbose Option:

Add a verbose mode that provides more detailed output about the search process, which can be useful for debugging or understanding what the program is doing.
Improved Pattern Matching:

Enhance the pattern matching functionality to support more complex patterns, not just simple wildcard extensions.
Integration with Standard Tools:

Provide options to output results in a format that can be easily piped into standard Linux tools like grep, awk, or sed for further processing.
Installation Script:

Provide an installation script to easily compile and install the program on the user's system.
Robust Error and Exception Handling:

Ensure all possible errors and exceptions are caught and handled gracefully, providing clear and helpful error messages to the user.
These improvements would make the program more flexible and convenient to use in a variety of scenarios typical in the Linux/Ubuntu command-line environment.
