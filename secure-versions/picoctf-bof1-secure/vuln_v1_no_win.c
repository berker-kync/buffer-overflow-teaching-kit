#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Version 1: Remove win() function
// Defense: If win() doesn't exist, ret2win attack can't work
// Note: This is still vulnerable to overflow, but there's no target to jump to

// Declare gets() to avoid warnings (it's deprecated)
char *gets(char *);

void vuln() {
    char buf[32];
    printf("Enter some text: ");
    gets(buf);  // Still vulnerable!
    printf("You entered: %s\n", buf);
}

int main() {
    vuln();
    return 0;
}

// NO win() function - this is the defense!
// Even if attacker overwrites return address,
// they can't jump to win() because it doesn't exist
