/*
 * Example 2: Loop with known bounds
 * Demonstrates value range propagation through loops
 */

#include <stdio.h>

int sum_array(int *arr, int size) {
    int sum = 0;

    // Size is always in range [1, 1000]
    for (int i = 0; i < size; i++) {
        sum += arr[i];

        // Loop counter i is bounded by size
        if (i >= 1000) {
            // This should be unreachable
            return -1;
        }
    }

    return sum;
}

int main(void) {
    int data[100];

    // Initialize array
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }

    int total = sum_array(data, 100);
    printf("Sum: %d\n", total);

    return 0;
}
