#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Version 3: All Defenses Combined
// Defense: Safe code + Stack canary + NX bit + Fortification
// Real-world secure programming

int bof(char *str)
{
    char buffer[24];
    
    // DEFENSE 1: Input validation
    size_t input_len = strlen(str);
    if (input_len >= sizeof(buffer)) {
        printf("Error: Input too long (%zu bytes). Maximum: %zu bytes\n",
               input_len, sizeof(buffer) - 1);
        return 0;
    }
    
    // DEFENSE 2: Safe copy
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    printf("Input processed: %s\n", buffer);
    return 1;
}

int main(int argc, char **argv)
{
    char str[517];
    FILE *badfile;

    badfile = fopen("badfile", "r");
    if (badfile == NULL) {
        printf("Error: Cannot open badfile\n");
        return 1;
    }
    
    size_t bytes_read = fread(str, sizeof(char), 517, badfile);
    str[bytes_read] = '\0';
    fclose(badfile);
    
    if (bof(str)) {
        printf("Returned Properly\n");
    } else {
        printf("Input rejected\n");
    }
    
    return 0;
}

// Compiled with:
// gcc -m32 -fstack-protector-all -D_FORTIFY_SOURCE=2 -z noexecstack -no-pie \
//     -o stack_v3 stack_v3_all_defenses.c
//
// Defenses:
// 1. Safe code (input validation + strncpy)
// 2. Stack canary (-fstack-protector-all)
// 3. Fortified functions (-D_FORTIFY_SOURCE=2)
// 4. Non-executable stack (-z noexecstack)
