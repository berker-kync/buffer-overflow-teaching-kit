#include <stdio.h>
#include <stdlib.h>

void bof(char *str) {
    char buffer[24];
    printf("Buffer address: %p\n", buffer);
}

int main() {
    char str[517];
    FILE *badfile = fopen("badfile", "r");
    if (badfile) {
        fread(str, sizeof(char), 517, badfile);
        fclose(badfile);
    }
    bof(str);
    return 0;
}
