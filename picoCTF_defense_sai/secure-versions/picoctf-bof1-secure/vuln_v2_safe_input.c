#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DEFENSE VERSION 2: use safe input instead of gets()
// This program does NOT have a win() function and
// also avoids buffer overflow by using fgets() with
// a maximum length equal to the buffer size.

void vuln() {
    char buf[32];

    printf("Enter some text: ");

    // fgets reads at most sizeof(buf) - 1 characters
    // and always adds a null terminator.
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("Input error.\n");
        exit(1);
    }

    // Remove trailing newline if present
    buf[strcspn(buf, "\n")] = '\0';

    printf("You entered: %s\n", buf);
}

int main(int argc, char **argv) {
    vuln();
    return 0;
}
