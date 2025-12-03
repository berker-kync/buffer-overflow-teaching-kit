#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Version 2: Stack Canary + Non-Executable Stack
// Defense: Compiler detects overflow + OS prevents shellcode execution
//
// NOTE: Code is INTENTIONALLY vulnerable
// Defenses come from compilation flags
//
// Compile with:
// gcc -m32 -fstack-protector-all -z noexecstack -no-pie -o stack_v2 stack_v2_canary_nx.c
//
// Two defenses:
// 1. Stack canary (-fstack-protector-all) - detects overflow
// 2. NX bit (-z noexecstack) - prevents shellcode execution

int bof(char *str)
{
    char buffer[24];
    strcpy(buffer, str);  // Still vulnerable!
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
    
    fread(str, sizeof(char), 517, badfile);
    fclose(badfile);
    
    bof(str);
    printf("Returned Properly\n");
    return 0;
}
