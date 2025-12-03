// Version 4: ASLR/PIE Enabled
// 
// IMPORTANT: This code is IDENTICAL to vuln_v3_canary.c
// The difference is in HOW we compile it:
//
// Version 3 compilation:
//   gcc -m32 -fstack-protector-all -no-pie -o vuln_v3 vuln_v3_canary.c
//   Result: win() is at FIXED address (e.g., 0x08049186)
//
// Version 4 compilation:
//   gcc -m32 -fstack-protector-all -pie -o vuln_v4 vuln_v4_aslr.c  
//   Result: win() is at RANDOM address each run
//
// This makes ret2win unreliable because attacker can't predict the address!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *gets(char *);

void win() {
    printf("Congratulations! You win!\n");
}

void vuln() {
    char buf[32];
    printf("Enter some text: ");
    gets(buf);              // still vulnerable on purpose
    printf("You entered: %s\n", buf);
}

int main(int argc, char **argv) {
    vuln();
    return 0;
}
