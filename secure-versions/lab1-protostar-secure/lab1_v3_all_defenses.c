#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

// Version 3: All Defenses Combined
// Defense: Safe code + Stack canary + Fortification
// Real-world secure programming
//
// Compiled with:
// gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -no-pie \
//     -o stack0_v3_all_defenses stack0_v3_all_defenses.c
//
// Defenses:
// 1. Safe code (input validation + fgets + strncpy)
// 2. Stack canary (-fstack-protector-all)
// 3. Fortified functions (-D_FORTIFY_SOURCE=2)

int main(int argc, char **argv)
{
    volatile int modified;
    char buffer[64];
    char input[256];
    
    modified = 0;
    
    printf("Enter input: ");
    
    // DEFENSE 1: Safe input
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error reading input\n");
        return 1;
    }
    
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
