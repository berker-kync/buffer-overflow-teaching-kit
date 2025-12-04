#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

// Version 1: Safe Code Practices
// Defense: Input validation + safe functions
// Prevents overflow at CODE level

int main(int argc, char **argv)
{
    volatile int modified;
    char buffer[64];
    char input[256];
    
    modified = 0;
    
    printf("Enter input: ");
    
    // DEFENSE 1: Use safe input function
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error reading input\n");
        return 1;
    }
    
    // Remove newline
    input[strcspn(input, "\n")] = 0;
    
    // DEFENSE 2: Input validation
    if (strlen(input) >= sizeof(buffer)) {
        printf("Input too long! Maximum %lu characters\n", sizeof(buffer) - 1);
        return 1;
    }
    
    // DEFENSE 3: Safe copy
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    if (modified != 0) {
        printf("you have changed the 'modified' variable\n");
    } else {
        printf("Try again?\n");
    }
    
    return 0;
}
