#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

// Version 2: Stack Canary Protection
// Defense: Compiler detects overflow
//
// NOTE: Code is INTENTIONALLY vulnerable
// Defense comes from compilation flags
//
// Compile with:
// gcc -m32 -fstack-protector-all -no-pie -o stack0_v2_canary stack0_v2_canary.c
//
// With -fstack-protector-all:
// 1. Compiler inserts canary before return address
// 2. Checks canary before function returns
// 3. Aborts if canary was overwritten

int main(int argc, char **argv)
{
    volatile int modified;
    char buffer[64];

    modified = 0;

    printf("Enter input: ");
    gets(buffer);  // Still vulnerable!

    if (modified != 0) {
        printf("you have changed the 'modified' variable\n");
    } else {
        printf("Try again?\n");
    }

    return 0;
}
