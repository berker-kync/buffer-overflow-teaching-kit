#include <stdio.h>
#include <stdlib.h>

void bof(char *str) {
    char buffer[24];
    printf("MYSHELL in this program: %p\n", getenv("MYSHELL"));
    printf("Buffer: %p\n", buffer);
}

int main() {
    char str[517];
    FILE *badfile = fopen("badfile", "r");
    if (badfile) {
        fread(str, 1, 517, badfile);
        fclose(badfile);
    }
    bof(str);
    return 0;
}
