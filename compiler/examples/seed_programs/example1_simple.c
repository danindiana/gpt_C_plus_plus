/*
 * Example 1: Simple program with potential dead code
 * This program demonstrates a case where value range information
 * can help the compiler optimize better.
 */

#include <stdio.h>
#include <stdlib.h>

int compute_value(int x) {
    int result = 0;

    // This condition is always true when x is in range [0, 100]
    if (x >= 0 && x <= 100) {
        result = x * 2;
    } else {
        // This branch could be marked as unreachable
        result = -1;
    }

    return result;
}

int main(int argc, char *argv[]) {
    int value = 42;

    if (argc > 1) {
        value = atoi(argv[1]);
        // Assume value is always in range [0, 100]
        if (value < 0 || value > 100) {
            value = 50; // Clamp to valid range
        }
    }

    int result = compute_value(value);
    printf("Result: %d\n", result);

    return 0;
}
