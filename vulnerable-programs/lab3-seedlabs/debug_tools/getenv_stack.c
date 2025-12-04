#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bof(char *str) {
    char buffer[24];
    strcpy(buffer, str);
    return 1;
}

int main(int argc, char **argv) {
    printf("MYSHELL for ./stack: %p\n", getenv("MYSHELL"));
    char str[517];
    FILE *badfile = fopen("badfile", "r");
    if (badfile) {
        fread(str, sizeof(char), 517, badfile);
        fclose(badfile);
    }
    bof(str);
    printf("Returned Properly\n");
    return 1;
}
