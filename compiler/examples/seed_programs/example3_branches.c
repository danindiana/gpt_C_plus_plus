/*
 * Example 3: Complex branching with dead code
 * Tests dead code elimination with multiple conditions
 */

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_INVALID = 2
} Status;

Status process_data(int value, bool flag) {
    Status status = STATUS_OK;

    // When value is known to be positive and flag is true,
    // some branches become unreachable
    if (value > 0) {
        if (flag) {
            status = STATUS_OK;
        } else {
            status = STATUS_ERROR;
        }
    } else {
        // If value is always positive, this is dead code
        status = STATUS_INVALID;
    }

    return status;
}

int main(void) {
    int value = 42;  // Always positive
    bool flag = true;

    Status result = process_data(value, flag);

    switch (result) {
        case STATUS_OK:
            printf("Success\n");
            break;
        case STATUS_ERROR:
            printf("Error\n");
            break;
        case STATUS_INVALID:
            // This case should never be reached
            printf("Invalid\n");
            break;
    }

    return 0;
}
