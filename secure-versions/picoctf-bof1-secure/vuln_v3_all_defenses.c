#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Version 3: All Defenses Combined
// Defense: Safe code + Stack canary + ASLR/PIE
// This represents REAL-WORLD secure programming

void win() {
    printf("Congratulations! You win!\n");
}

void vuln() {
    char buf[32];
    char input[256];

    printf("Enter some text: ");
    
    // DEFENSE 1: Safe input
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error reading input\n");
        return;
    }
    
    input[strcspn(input, "\n")] = 0;
    
    // DEFENSE 2: Input validation
    if (strlen(input) >= sizeof(buf)) {
        printf("Input too long! Maximum %lu characters\n", sizeof(buf) - 1);
        return;
    }
    
    // DEFENSE 3: Safe copy
    strncpy(buf, input, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    
    printf("You entered: %s\n", buf);
}

int main(int argc, char **argv) {
    vuln();
    return 0;
}

// Compiled with:
// gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -pie -o vuln_v3 vuln_v3_all_defenses.c
//
// Defenses active:
// 1. Safe code practices (in code above)
// 2. Stack canary (-fstack-protector-all)
// 3. Fortified functions (-D_FORTIFY_SOURCE=2)
// 4. ASLR/PIE (-pie) - randomizes addresses
