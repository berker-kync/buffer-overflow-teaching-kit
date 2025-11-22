#include <stdio.h>
#include <string.h>

int bof(char *str) {
    char buffer[24];
    printf("Buffer at: %p\n", buffer);
    printf("Return addr should be at: %p\n", __builtin_frame_address(0) + 4);
    strcpy(buffer, str);
    return 1;
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
