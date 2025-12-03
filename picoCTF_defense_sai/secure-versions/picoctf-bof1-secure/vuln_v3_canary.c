#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
char *gets(char *);   // <-- add this line
// Version 3: same vulnerable code,
// but we will compile it WITH stack canaries.

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
