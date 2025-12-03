#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *gets(char *);

// Version 2: Stack Canary (Compiler Protection)
// Defense: Compiler detects stack overflow
// 
// NOTE: Code is INTENTIONALLY vulnerable
// The defense comes from compilation flags
//
// Compile with: gcc -m32 -fstack-protector-all -no-pie -o vuln_v2 vuln_v2_canary.c
//
// The compiler automatically:
// 1. Places a canary value before return address
// 2. Checks canary before function returns
// 3. Aborts if canary was overwritten

void win() {
    printf("Congratulations! You win!\n");
}

void vuln() {
    char buf[32];
    printf("Enter some text: ");
    gets(buf);  // Still vulnerable!
    printf("You entered: %s\n", buf);
}

int main(int argc, char **argv) {
    vuln();
    return 0;
}
